
#include <gtest/gtest.h>
#include "mixing_fab_tests.h"


using pyne::nucname::id;
using cyclus::Composition;
using cyclus::CompMap;
using cyclus::compmath::Normalize;
using cyclus::compmath::Add;

using cyclus::Material;
using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::toolkit::MatQuery;

namespace cycamore {
  
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  Composition::Ptr c_pustream() {
    CompMap m;
    m[id("pu239")] = 100;
    m[id("pu240")] = 10;
    m[id("pu241")] = 1;
    m[id("pu242")] = 1;
    return Composition::CreateFromMass(m);
  };
  
  Composition::Ptr c_natu() {
    CompMap m;
    m[id("u235")] =  .007;
    m[id("u238")] =  .993;
    return Composition::CreateFromMass(m);
  };
  
  Composition::Ptr c_uox() {
    CompMap m;
    m[id("u235")] = 0.04;
    m[id("u238")] = 0.96;
    return Composition::CreateFromMass(m);
  };

  void MixingFabTest::SetUp() {
    mf_facility_ = new MixingFab(tc_.get());
    InitParameters();
    SetUpMixingFab();
  }
  
  void MixingFabTest::TearDown() {
    delete mf_facility_;
  }
  
  void MixingFabTest::InitParameters(){
    in_com.push_back("in_c1"); in_frac.push_back(0.80); in_cap.push_back(3);
    in_com.push_back("in_c2"); in_frac.push_back(0.15); in_cap.push_back(2);
    in_com.push_back("in_c3"); in_frac.push_back(0.05); in_cap.push_back(1);
    
    out_com = "out_com"; out_cap = 3;
    
    throughput = 20;
  }
  
  void MixingFabTest::SetUpMixingFab(){
    mf_facility_->in_commods = in_com;
    mf_facility_->mixing_frac = in_frac;
    mf_facility_->in_buf_size = in_cap;
    
    mf_facility_->out_commod = out_com;
    mf_facility_->output_buf_size = out_cap;
    
    mf_facility_->throughput = throughput;
  }
  
  void MixingFabTest::TestInitState(MixingFab* fac){
    for (int i = 0; i<in_com.size(); i++) {
      EXPECT_EQ(in_com[i], fac->in_commods[i]);
      EXPECT_EQ(in_frac[i], fac->mixing_frac[i]);
      EXPECT_EQ(in_cap[i], fac->in_buf_size[i]);
    }
    
    EXPECT_EQ(out_com, fac->out_commod);
    EXPECT_EQ(out_cap, fac->output_buf_size);
    
    EXPECT_EQ(throughput, fac->throughput);
  }
  
  void MixingFabTest::SetInputInv(cycamore::MixingFab *fac, std::vector< Material::Ptr > mat){
    for (int i = 0; i<in_com.size(); i++) {
      fac->streambufs[in_com[i]].Push(mat[i]) ;
    }
  }
  
  void MixingFabTest::TestBuffers(MixingFab* fac,
                                  std::vector<double> in_inv,
                                  double out_inv){
    for (int i = 0; i<in_com.size(); i++) {
      EXPECT_EQ(in_inv[i], fac->streambufs[in_com[i]].quantity());
    }
    EXPECT_EQ(out_inv, fac->output.quantity());
  }
  
  void MixingFabTest::TestOutputComposition(MixingFab* fac, cyclus::CompMap v){
    
    cyclus::toolkit::ResBuf<cyclus::Material>* buffer = &fac->output;
    Material::Ptr final_mat = cyclus::ResCast<Material>(buffer->PopBack());
    cyclus::CompMap final_comp = final_mat->comp()->mass();
    Normalize(&v,1);
    Normalize(&final_comp,1);
    
    EXPECT_EQ(final_comp,v);
    
  }
  
  
  
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  TEST_F(MixingFabTest, InitialState) {
    // Test things about the initial state of the facility here
    TestInitState(mf_facility_);
  }

  
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // multiple input streams can be correctly requested and used as
  //  material inventory.
  TEST_F(MixingFabTest, MultipleFissStreams) {
    std::string config =
    "<in_commods> <val>stream1</val> <val>stream2</val> <val>stream3</val> </in_commods>"
    "<in_buf_size> <val>2.5</val> <val>3</val> <val>5</val></in_buf_size>"
    "<mixing_frac> <val>0.8</val> <val>0.15</val> <val>0.05</val></mixing_frac>"
    ""
    "<out_commod>dummyout</out_commod>"
    "<outputbuf_size>0</outputbuf_size>"
    "<throughput>0</throughput>"
    ;
    
    int simdur = 1;
    cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:MixingFab"), config, simdur);
    sim.AddSource("stream1").recipe("unatstream").capacity(1).Finalize();
    sim.AddSource("stream2").recipe("uoxstream").capacity(1).Finalize();
    sim.AddSource("stream3").recipe("pustream").capacity(1).Finalize();
    sim.AddRecipe("unatstream", c_natu());
    sim.AddRecipe("uoxstream", c_pustream());
    sim.AddRecipe("pustream", c_uox());
    int id = sim.Run();
    
    QueryResult qr = sim.db().Query("Transactions", NULL);
    EXPECT_EQ(3, qr.rows.size());
    
    std::vector<Cond> conds;
    conds.push_back(Cond("Commodity", "==", std::string("stream1")));
    qr = sim.db().Query("Transactions", &conds);
    EXPECT_EQ(1, qr.rows.size());
    
    conds[0] = Cond("Commodity", "==", std::string("stream2"));
    qr = sim.db().Query("Transactions", &conds);
    EXPECT_EQ(1, qr.rows.size());
    
    conds[0] = Cond("Commodity", "==", std::string("stream3"));
    qr = sim.db().Query("Transactions", &conds);
    EXPECT_EQ(1, qr.rows.size());
  }
  
  
    // throughput is properly restricted when faced with many fuel
    // requests and with ample material inventory.
  TEST_F(MixingFabTest, Mixing) {
    throughput = 1e200;
    SetUpMixingFab();

    std::vector<Material::Ptr> mat;
    mat.push_back(Material::CreateUntracked(in_frac[0], c_natu()));
    mat.push_back(Material::CreateUntracked(in_frac[1], c_pustream()));
    mat.push_back(Material::CreateUntracked(in_frac[2], c_uox()));

    
    SetInputInv(mf_facility_, mat);
    mf_facility_->Tick();
    
    
    
    CompMap v_0 = c_natu()->mass();
    Normalize(&v_0, in_frac[0]);
    CompMap v_1 = c_pustream()->mass();
    Normalize(&v_1, in_frac[1]);
    CompMap v_2 = c_uox()->mass();
    Normalize(&v_2, in_frac[2]);
    CompMap v = v_0;
    v = Add( v, v_1);
    v = Add( v, v_2);
    
    TestOutputComposition(mf_facility_, v);
    
    }
} // namespace cycamore
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  


