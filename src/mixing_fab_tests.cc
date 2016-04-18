#include "mixing_fab.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"
#include "equality_helpers.h"
#include "cyclus.h"


#include <gtest/gtest.h>

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
  
  
  class MixingFabTest : public ::testing::Test {
    
    
  public:
    typedef cyclus::toolkit::ResBuf<cyclus::Material> InvBuffer;
    
    cyclus::TestContext tc_;
    MixingFab* mf_facility_;
    
    virtual void SetUp(){
      mf_facility_ = new MixingFab(tc_.get());
      
      std::vector<std::string> in_com_ = { "in_c1", "in_c2", "in_c3" };
      SetStream_comds(in_com_);

      std::vector<double> in_cap_ = { 30, 20, 10 };
      SetStream_capacity(in_cap_);
      
      SetOutStream_comds("out_com");

    }
    virtual void TearDown() { delete mf_facility_; }
    
    std::vector<std::string> in_com;
    std::vector<double> in_frac;
    std::vector<double> in_cap;
    
    
    std::string out_com;
    double out_cap;
    
    double throughput;
    
    void SetThroughput(double thpt) { throughput = thpt; mf_facility_->throughput = thpt;}
    
    void SetStream_comds(std::vector<std::string> comds)   { in_com  = comds;  mf_facility_->in_commods   = comds; }
    void SetStream_ratio(std::vector<double> ratio)        { in_frac = ratio;  mf_facility_->mixing_ratio = ratio; }
    void SetStream_capacity(std::vector<double> cap)       { in_cap  = cap;    mf_facility_->in_buf_size  = cap; }
    
    void SetOutStream_comds(std::string com)  { out_com = com; mf_facility_->out_commod      = com; }
    void SetOutStream_capacity(double cap)    { out_cap = cap; mf_facility_->out_buf_size = cap; }
    void SetInputInv(std::vector< Material::Ptr > mat){
      for (int i = 0; i < in_com.size(); i++) {
        mf_facility_->streambufs[in_com[i]].Push(mat[i]) ;
      }
    }
    
    std::vector<std::string>  GetStream_comds() { return mf_facility_->in_commods; }
    std::vector<double> GetStream_ratio()       { return mf_facility_->mixing_ratio; }
    std::vector<double> GetStream_capacity()    { return mf_facility_->in_buf_size; }
    
    std::string GetOutStream_comds()  { return mf_facility_->out_commod; }
    double GetOutStream_capacity()    { return mf_facility_->out_buf_size; }
    double GetThroughput()    { return mf_facility_->throughput; }
    
    
    InvBuffer* GetOutPutBuffer() { return &mf_facility_->output;}
    std::map<std::string, InvBuffer > GetStreamBuffer() { return mf_facility_->streambufs;}
  
  };
  


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Checking that ratios correctly default to 1/N.
  TEST_F(MixingFabTest, StreamDefaultRatio) {
  
    SetOutStream_capacity(50);
    
    SetThroughput(1e200);
    
    mf_facility_->EnterNotify();
    double ext_val = 1/3.;
    std::vector<double> strm_ratio_ = GetStream_ratio();
    
    //
    for (int i = 0; i < in_com.size(); i++) {
      EXPECT_DOUBLE_EQ(ext_val, strm_ratio_[i]);
    }
  }
    // Test things about the mixing ratio normalisation.
  TEST_F(MixingFabTest, StreamRatio) {
   
    // Checking renormalisation when sum of ratio is grester tham 1.
    std::vector<double> in_cap_ = { 30, 20, 10 };
    std::vector<double> in_frac_ = { 2, 1, 5 };
    in_cap_ = { 30, 20, 10 };
    
    SetStream_ratio(in_frac_);
    SetStream_capacity(in_cap_);
    
    SetOutStream_capacity(50);
    
    SetThroughput(1e200);
    
    mf_facility_->EnterNotify();
    
    std::vector<double> strm_ratio_ = GetStream_ratio();
    double sum = 0;
    for (int i = 0; i < in_com.size(); i++) {
      sum += strm_ratio_[i];
    }

    EXPECT_DOUBLE_EQ(sum, 1) << "Ratios normalized incorrectly: want 1, got " << sum;

    // Checking renormalisation when sum of ratio is smaller tham 1.
    in_frac_ = { 0.1, 0.2, 0.5 };
    
    SetStream_ratio(in_frac_);
    
    SetOutStream_capacity(50);
    
    SetThroughput(1e200);
    
    mf_facility_->EnterNotify();
    
    strm_ratio_ = GetStream_ratio();
    sum = 0;
    for (int i = 0; i < in_com.size(); i++) {
      sum += strm_ratio_[i];
    }

    EXPECT_DOUBLE_EQ(sum, 1) << "Ratios normalized incorrectly: want 1, got " << sum;
  
    
  }

  // Check the correct mixing composition
  TEST_F(MixingFabTest, MixingComposition) {
    
    std::vector<double> in_frac_ = { 0.80, 0.15, 0.05 };
    
    SetStream_ratio(in_frac_);
    
    SetOutStream_capacity(50);
    
    SetThroughput(1e200);
    
    std::vector<Material::Ptr> mat;
    mat.push_back(Material::CreateUntracked(in_cap[0], c_natu()));
    mat.push_back(Material::CreateUntracked(in_cap[1], c_pustream()));
    mat.push_back(Material::CreateUntracked(in_cap[2], c_uox()));
    
    SetInputInv(mat);
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
    
    InvBuffer* buffer = GetOutPutBuffer();
    Material::Ptr final_mat = cyclus::ResCast<Material>(buffer->PopBack());
    cyclus::CompMap final_comp = final_mat->comp()->mass();
    
    Normalize(&v,1);
    Normalize(&final_comp,1);
    
    cyclus::CompMap sum_v = Add(v, final_comp);
    
    CompMap::iterator it;
    for(it = sum_v.begin(); it != sum_v.end(); it ++){
      EXPECT_DOUBLE_EQ(final_comp[it->first],v[it->first]) << "Unexpected difference on nuclide " << it->first << ".";
    }
  }
  
  
  
  // Check the throughput constrain
  TEST_F(MixingFabTest, Throughput) {
    
    std::vector<double> in_frac_ = { 0.80, 0.15, 0.05 };
    
    SetStream_ratio(in_frac_);
    
    SetOutStream_capacity(50);
    
    SetThroughput(0.5);
    
    std::vector<Material::Ptr> mat;
    mat.push_back(Material::CreateUntracked(in_cap[0], c_natu()));
    mat.push_back(Material::CreateUntracked(in_cap[1], c_pustream()));
    mat.push_back(Material::CreateUntracked(in_cap[2], c_uox()));
    SetInputInv(mat);
    
    mf_facility_->Tick();
    
    std::vector<double> cap;
    for(int i = 0; i < in_com.size(); i++ ){
      cap.push_back(in_cap[i]-0.5*in_frac[i]);
    }
    
    std::map<std::string, InvBuffer> streambuf = GetStreamBuffer();
    
    for (int i = 0; i < in_com.size(); i++) {
      std::string buf_com = in_com[i];
      double buf_size = in_cap[i];
      double buf_ratio = in_frac[i];
      double buf_inv = streambuf[buf_com].quantity();
      
      // checking that each input buf was reduce of the correct amount (constrained by the throughput"
      EXPECT_EQ( buf_size - 0.5*buf_ratio, buf_inv) << " one (or more) input buffer inventory was not drawn from in the correct ratio.";
    }
    
    
    // output buffer size should be equal to the throuput size
    EXPECT_EQ(throughput, GetOutPutBuffer()->quantity()) << " mixing was not correctly constrained by throughput.";
    
  }

  // multiple input streams can be correctly requested and used as
  //  material inventory.
  TEST(MixingFabTests, MultipleFissStreams) {
    std::string config =
    "<in_commods> <val>stream1</val> <val>stream2</val> <val>stream3</val> </in_commods>"
    "<in_buf_size> <val>2.5</val> <val>3</val> <val>5</val></in_buf_size>"
    "<mixing_ratio> <val>0.8</val> <val>0.15</val> <val>0.05</val></mixing_ratio>"
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
    
    // Checking the number of transaction is as expected 3.
    QueryResult qr = sim.db().Query("Transactions", NULL);
    EXPECT_EQ(3, qr.rows.size());
    
    
    
    // Checking that all input stream get one transaction each.
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

  
  
  // multiple input streams can be correctly requested and used as
  //  material inventory.
  TEST(MixingFabTests, CompleteMixingProcess) {
    std::string config =
    "<in_commods> <val>stream1</val> <val>stream2</val> <val>stream3</val> </in_commods>"
    "<in_buf_size> <val>2.5</val> <val>3</val> <val>5</val></in_buf_size>"
    "<mixing_ratio> <val>0.8</val> <val>0.15</val> <val>0.05</val></mixing_ratio>"
    ""
    "<out_commod>mixedstream</out_commod>"
    "<outputbuf_size>10</outputbuf_size>"
    "<throughput>1</throughput>"
    ;
    
    int simdur = 2;
    cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:MixingFab"), config, simdur);
    sim.AddSource("stream1").recipe("unatstream").capacity(1).Finalize();
    sim.AddSource("stream2").recipe("uoxstream").capacity(1).Finalize();
    sim.AddSource("stream3").recipe("pustream").capacity(1).Finalize();
    sim.AddRecipe("unatstream", c_natu());
    sim.AddRecipe("uoxstream", c_pustream());
    sim.AddRecipe("pustream", c_uox());
    
    sim.AddSink("mixedstream").capacity(10).Finalize();
    int id = sim.Run();

    // Checking that all input stream get one transaction each.
    std::vector<Cond> conds;
    conds.push_back(Cond("Commodity", "==", std::string("mixedstream")));
    QueryResult qr = sim.db().Query("Transactions", &conds);
    EXPECT_EQ(1, qr.rows.size());
    
    
    Material::Ptr m = sim.GetMaterial(qr.GetVal<int>("ResourceId"));
    EXPECT_DOUBLE_EQ(1., m->quantity());

  }

  
  
} // namespace cycamore
