#include "fuel_fab.h"

#include <gtest/gtest.h>
#include <sstream>
#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::CompMap;
using cyclus::Material;
using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::toolkit::MatQuery;

namespace cycamore {

Composition::Ptr c_uox() {
  CompMap m;
  m[id("u235")] = 0.04;
  m[id("u238")] = 0.96;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_mox() {
  CompMap m;
  m[id("u235")] = .7;
  m[id("u238")] = 100;
  m[id("pu239")] = 3.3;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_natu() {
  CompMap m;
  m[id("u235")] =  .007;
  m[id("u238")] =  .993;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustream() {
  CompMap m;
  m[id("pu239")] = 100;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustreamlow() {
  CompMap m;
  m[id("pu239")] = 80;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustreambad() {
  CompMap m;
  m[id("pu239")] = 10;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_water() {
  CompMap m;
  m[id("O16")] =  1;
  m[id("H1")] =  2;
  return Composition::CreateFromAtom(m);
};

TEST(FuelFabTests, CosiWeight) {
  cyclus::Env::SetNucDataPath();
  CompMap m;
  m[942390000] = 1;
  Composition::Ptr c = Composition::CreateFromMass(m);
  double w = CosiWeight(c, "thermal");
  EXPECT_DOUBLE_EQ(1.0, w);

  m.clear();
  m[922380000] = 1;
  c = Composition::CreateFromMass(m);
  w = CosiWeight(c, "thermal");
  EXPECT_DOUBLE_EQ(0.0, w);

  m.clear();
  m[942390000] = 1;
  c = Composition::CreateFromMass(m);
  w = CosiWeight(c, "fission_spectrum_ave");
  EXPECT_DOUBLE_EQ(1.0, w);

  m.clear();
  m[922380000] = 1;
  c = Composition::CreateFromMass(m);
  w = CosiWeight(c, "fission_spectrum_ave");
  EXPECT_DOUBLE_EQ(0.0, w);

  m.clear();
  m[922380000] = 1;
  m[942390000] = 1;
  c = Composition::CreateFromAtom(m);
  w = CosiWeight(c, "thermal");
  EXPECT_DOUBLE_EQ(0.5, w) << "might be using mass-fractions instead of atom";

  m.clear();
  m[922380000] = 1;
  m[942390000] = 1;
  c = Composition::CreateFromAtom(m);
  w = CosiWeight(c, "fission_spectrum_ave");
  EXPECT_DOUBLE_EQ(0.5, w) << "might be using mass-fractions instead of atom";

  m.clear();
  m[922380000] = 1;
  m[942390000] = 1;
  m[922350000] = 1;
  c = Composition::CreateFromAtom(m);
  double w_therm = CosiWeight(c, "thermal");
  double w_fast = CosiWeight(c, "fission_spectrum_ave");
  EXPECT_GT(w_therm, w_fast);
}

TEST(FuelFabTests, HighFrac) {
  double w_fill = CosiWeight(c_natu(), "thermal");
  double w_fiss = CosiWeight(c_pustream(), "thermal");
  double w_target = CosiWeight(c_uox(), "thermal");

  EXPECT_THROW(HighFrac(w_fiss, w_target, w_fill), cyclus::ValueError);
  EXPECT_THROW(HighFrac(w_target, w_fill, w_fiss), cyclus::ValueError);
  EXPECT_THROW(HighFrac(w_target, w_fiss, w_fill), cyclus::ValueError);

  double f = HighFrac(w_fill, w_target, w_fiss);
  EXPECT_DOUBLE_EQ((w_target-w_fill)/(w_fiss-w_fill), f);
}

TEST(FuelFabTests, LowFrac) {
  double w_fill = CosiWeight(c_natu(), "thermal");
  double w_fiss = CosiWeight(c_pustream(), "thermal");
  double w_target = CosiWeight(c_uox(), "thermal");

  EXPECT_THROW(LowFrac(w_fiss, w_target, w_fill), cyclus::ValueError);
  EXPECT_THROW(LowFrac(w_target, w_fill, w_fiss), cyclus::ValueError);
  EXPECT_THROW(LowFrac(w_target, w_fiss, w_fill), cyclus::ValueError);

  double f = LowFrac(w_fill, w_target, w_fiss);
  EXPECT_DOUBLE_EQ((w_fiss-w_target)/(w_fiss-w_fill), f);
}

TEST(FuelFabTests, ValidWeights) {
  double w_fill = CosiWeight(c_natu(), "thermal");
  double w_fiss = CosiWeight(c_pustream(), "thermal");
  double w_target = CosiWeight(c_uox(), "thermal");

  EXPECT_EQ(true, ValidWeights(w_fill, w_target, w_fiss));
  EXPECT_EQ(false, ValidWeights(w_fiss, w_target, w_fill));
  EXPECT_EQ(false, ValidWeights(w_target, w_fill, w_fiss));
  EXPECT_EQ(false, ValidWeights(w_fiss, w_fill, w_target));
  EXPECT_EQ(false, ValidWeights(w_target, w_fiss, w_fill));
  EXPECT_EQ(false, ValidWeights(w_fill, w_fiss, w_target));
}

TEST(FuelFabTests, Template) {
  std::string config = 
     "<fill_commod>commod</fill_commod>"
     "<fill_recipe>recipe</fill_recipe>"
     "<fill_size>1</fill_size>"
     ""
     "<fiss_commods> <val>commod</val> <val>commod</val> </fiss_commods>"
     "<fiss_size>1</fiss_size>"
     ""
     "<topup_commod>commod</topup_commod>"
     "<topup_recipe>recipe</topup_recipe>"
     "<topup_size>1</topup_size>"
     ""
     "<outcommod>commod</outcommod>"
     "<spectrum>thermal</spectrum>"
     "<throughput>1</throughput>"
     ;

  int simdur = 50;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:FuelFab"), config, simdur);
  //sim.AddSource("enriched_u").Finalize();
  //sim.AddRecipe("lwr_fresh", c_uox());
  //sim.AddRecipe("lwr_spent", c_spentuox());
  //int id = sim.Run();

  //QueryResult qr = sim.db().Query("Transactions", NULL);
  //EXPECT_EQ(simdur, qr.rows.size()) << "failed to order+run on fresh fuel inside 1 time step";
}

// request (and receive) a specific recipe for fissile stream correctly.
TEST(FuelFabTests, FissRecipe) {
  std::string config = 
     "<fill_commod>dummy</fill_commod>"
     "<fill_recipe>natu</fill_recipe>"
     "<fill_size>1</fill_size>"
     ""
     "<fiss_commods> <val>stream1</val> <val>stream2</val> <val>stream3</val> </fiss_commods>"
     "<fiss_size>2.5</fiss_size>"
     "<fiss_recipe>spentuox</fiss_recipe>"
     ""
     "<outcommod>dummyout</outcommod>"
     "<spectrum>thermal</spectrum>"
     "<throughput>0</throughput>"
     ;

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:FuelFab"), config, simdur);
  sim.AddSource("stream1").Finalize();
  sim.AddRecipe("spentuox", c_pustream());
  sim.AddRecipe("natu", c_natu());
  int id = sim.Run();

  int resid = sim.db().Query("Transactions", NULL).GetVal<int>("ResourceId");
  CompMap got = sim.GetMaterial(resid)->comp()->mass();
  CompMap want = c_pustream()->mass();
  cyclus::compmath::Normalize(&got);
  cyclus::compmath::Normalize(&want);
  CompMap::iterator it;
  for (it = want.begin(); it != want.end(); ++it) {
    EXPECT_DOUBLE_EQ(it->second, got[it->first]) << "nuclide qty off: " << pyne::nucname::name(it->first);
  }
}

// multiple fissile streams can be correctly requested and used as
// fissile material inventory.
TEST(FuelFabTests, MultipleFissStreams) {
  std::string config = 
     "<fill_commod>dummy</fill_commod>"
     "<fill_recipe>natu</fill_recipe>"
     "<fill_size>1</fill_size>"
     ""
     "<fiss_commods> <val>stream1</val> <val>stream2</val> <val>stream3</val> </fiss_commods>"
     "<fiss_size>2.5</fiss_size>"
     ""
     "<outcommod>dummyout</outcommod>"
     "<spectrum>thermal</spectrum>"
     "<throughput>0</throughput>"
     ;

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:FuelFab"), config, simdur);
  sim.AddSource("stream1").recipe("spentuox").capacity(1).Finalize();
  sim.AddSource("stream2").recipe("spentuox").capacity(1).Finalize();
  sim.AddSource("stream3").recipe("spentuox").capacity(1).Finalize();
  sim.AddRecipe("spentuox", c_pustream());
  sim.AddRecipe("natu", c_natu());
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

// fissile stream preferences can be specified.
TEST(FuelFabTests, FissStreamPrefs) {
  FAIL() << "not implemented";
}

// fill, fiss, and topup inventories are all requested for and
// filled as expected
TEST(FuelFabTests, FillInventories) {
  FAIL() << "not implemented";
}

// throughput is properly restricted when faced with many fuel
// requests and with ample material inventory.
TEST(FuelFabTests, ThroughputLimit) {
  FAIL() << "not implemented";
}

// supplied fuel has proper equivalence weights as requested.
TEST(FuelFabTests, CorrectMixing) {
  FAIL() << "not implemented";
}

// fuel is requested requiring more filler than is available with plenty of
// fissile.
TEST(FuelFabTests, FillConstrained) {
  FAIL() << "not implemented";
}

// fuel is requested requiring more fissile material than is available with
// plenty of filler.
TEST(FuelFabTests, FissConstrained) {
  FAIL() << "not implemented";
}

// swap to topup inventory because fissile has too low reactivity.
TEST(FuelFabTests, SwapTopup) {
  FAIL() << "not implemented";
}

// swap to topup inventory but are limited by topup inventory quantity.
TEST(FuelFabTests, SwapTopup_TopupConstrained) {
  FAIL() << "not implemented";
}

// swap to topup inventory but are limited by fissile (used as filler)
// inventory.
TEST(FuelFabTests, SwapTopup_FissConstrained) {
  FAIL() << "not implemented";
}

} // namespace cycamore

