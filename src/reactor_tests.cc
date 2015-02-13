#include <gtest/gtest.h>

#include <sstream>

#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::toolkit::MatQuery;

namespace cycamore {

Composition::Ptr c_uox() {
  cyclus::CompMap m;
  m[id("u235")] = 0.04;
  m[id("u238")] = 0.96;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_mox() {
  cyclus::CompMap m;
  m[id("u235")] = .7;
  m[id("u238")] = 100;
  m[id("pu239")] = 3.3;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_spentuox() {
  cyclus::CompMap m;
  m[id("u235")] =  .8;
  m[id("u238")] =  100;
  m[id("pu239")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_spentmox() {
  cyclus::CompMap m;
  m[id("u235")] =  .2;
  m[id("u238")] =  100;
  m[id("pu239")] = .9;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_water() {
  cyclus::CompMap m;
  m[id("O16")] =  1;
  m[id("H1")] =  2;
  return Composition::CreateFromAtom(m);
};

// Test that with a zero refuel_time and a zero capacity fresh fuel buffer
// (the default), fuel can be ordered and the cycle started with no time step
// delay.
TEST(ReactorTests, JustInTimeOrdering) {
  std::string config = 
     "  <fuel_inrecipes>  <val>lwr_fresh</val>  </fuel_inrecipes>  "
     "  <fuel_outrecipes> <val>lwr_spent</val>  </fuel_outrecipes>  "
     "  <fuel_incommods>  <val>enriched_u</val> </fuel_incommods>  "
     "  <fuel_outcommods> <val>waste</val>      </fuel_outcommods>  "
     "  <fuel_prefs>      <val>1.0</val>        </fuel_prefs>  "
     ""
     "  <cycle_time>1</cycle_time>  "
     "  <refuel_time>0</refuel_time>  "
     "  <assem_size>300</assem_size>  "
     "  <n_assem_core>1</n_assem_core>  "
     "  <n_assem_batch>1</n_assem_batch>  ";

  int simdur = 50;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Reactor"), config, simdur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddRecipe("lwr_fresh", c_uox());
  sim.AddRecipe("lwr_spent", c_spentuox());
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(simdur, qr.rows.size()) << "failed to order+run on fresh fuel inside 1 time step";
}

// The user can optionally omit fuel preferences.  In the case where
// preferences are adjusted, the ommitted preference vector must be populated
// with default values - if it wasn't then preferences won't be adjusted
// correctly and the reactor could segfault.  Check that this doesn't happen.
TEST(ReactorTests, PrefChange) {
  // it is important that the fuel_prefs not be present in the config below.
  std::string config = 
     "  <fuel_inrecipes>  <val>lwr_fresh</val>  </fuel_inrecipes>  "
     "  <fuel_outrecipes> <val>lwr_spent</val>  </fuel_outrecipes>  "
     "  <fuel_incommods>  <val>enriched_u</val> </fuel_incommods>  "
     "  <fuel_outcommods> <val>waste</val>      </fuel_outcommods>  "
     ""
     "  <cycle_time>1</cycle_time>  "
     "  <refuel_time>0</refuel_time>  "
     "  <assem_size>300</assem_size>  "
     "  <n_assem_core>1</n_assem_core>  "
     "  <n_assem_batch>1</n_assem_batch>  "
     ""
     "  <pref_change_times>   <val>25</val>         </pref_change_times>"
     "  <pref_change_commods> <val>enriched_u</val> </pref_change_commods>"
     "  <pref_change_values>  <val>-1</val>         </pref_change_values>";

  int simdur = 50;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Reactor"), config, simdur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddRecipe("lwr_fresh", c_uox());
  sim.AddRecipe("lwr_spent", c_spentuox());
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(25, qr.rows.size()) << "failed to adjust preferences properly";
}

// tests that the reactor handles requesting multiple types of fuel correctly
// - with proper inventory constraints, etc.
TEST(ReactorTests, MultiFuelMix) {
  FAIL() << "not implemented";
}

// tests that the reactor halts operation when it has no more room in its
// spent fuel inventory buffer.
TEST(ReactorTests, FullSpentInventory) {
  FAIL() << "not implemented";
}

// tests that the reactor cycle is delayed as expected when it is unable to
// acquire fuel in time for the next cycle start.
TEST(ReactorTests, FuelShortage) {
  FAIL() << "not implemented";
}

// tests that the refueling period between cycle end and start of the next
// cycle is honored.
TEST(ReactorTests, RefuelTimes) {
  FAIL() << "not implemented";
}

// tests that the correct number of assemblies are popped from the core each
// cycle.
TEST(ReactorTests, BatchSizes) {
  FAIL() << "not implemented";
}

TEST(ReactorTests, RecipeChange) {
  // it is important that the fuel_prefs not be present in the config below.
  std::string config = 
     "  <fuel_inrecipes>  <val>lwr_fresh</val>  </fuel_inrecipes>  "
     "  <fuel_outrecipes> <val>lwr_spent</val>  </fuel_outrecipes>  "
     "  <fuel_incommods>  <val>enriched_u</val> </fuel_incommods>  "
     "  <fuel_outcommods> <val>waste</val>      </fuel_outcommods>  "
     ""
     "  <cycle_time>1</cycle_time>  "
     "  <refuel_time>0</refuel_time>  "
     "  <assem_size>300</assem_size>  "
     "  <n_assem_core>1</n_assem_core>  "
     "  <n_assem_batch>1</n_assem_batch>  "
     ""
     "  <recipe_change_times>   <val>25</val>         <val>35</val>         </recipe_change_times>"
     "  <recipe_change_commods> <val>enriched_u</val> <val>enriched_u</val> </recipe_change_commods>"
     "  <recipe_change_in>      <val>water</val>      <val>water</val>      </recipe_change_in>"
     "  <recipe_change_out>     <val>lwr_spent</val>  <val>water</val>      </recipe_change_out>";

  int simdur = 50;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Reactor"), config, simdur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddSink("waste").Finalize();
  sim.AddRecipe("lwr_fresh", c_uox());
  sim.AddRecipe("lwr_spent", c_spentuox());
  sim.AddRecipe("water", c_water());
  int aid = sim.Run();

  std::vector<Cond> conds;
  QueryResult qr;

  // check that received recipe is not water
  conds.clear();
  conds.push_back(Cond("Time", "==", 24));
  conds.push_back(Cond("ReceiverId", "==", aid));
  qr = sim.db().Query("Transactions", &conds);
  MatQuery mq = MatQuery(sim.GetMaterial(qr.GetVal<int>("ResourceId")));

  EXPECT_TRUE(0 < mq.qty());
  EXPECT_TRUE(0 == mq.mass(id("H1")));

  // check that received recipe changed to water
  conds.clear();
  conds.push_back(Cond("Time", "==", 26));
  conds.push_back(Cond("ReceiverId", "==", aid));
  qr = sim.db().Query("Transactions", &conds);
  mq = MatQuery(sim.GetMaterial(qr.GetVal<int>("ResourceId")));

  EXPECT_TRUE(0 < mq.qty());
  EXPECT_TRUE(0 < mq.mass(id("H1")));

  // check that sent recipe is not water
  conds.clear();
  conds.push_back(Cond("Time", "==", 34));
  conds.push_back(Cond("SenderId", "==", aid));
  qr = sim.db().Query("Transactions", &conds);
  mq = MatQuery(sim.GetMaterial(qr.GetVal<int>("ResourceId")));

  EXPECT_TRUE(0 < mq.qty());
  EXPECT_TRUE(0 == mq.mass(id("H1")));

  // check that sent recipe changed to water
  conds.clear();
  conds.push_back(Cond("Time", "==", 36));
  conds.push_back(Cond("SenderId", "==", aid));
  qr = sim.db().Query("Transactions", &conds);
  mq = MatQuery(sim.GetMaterial(qr.GetVal<int>("ResourceId")));

  EXPECT_TRUE(0 < mq.qty());
  EXPECT_TRUE(0 < mq.mass(id("H1")));
}

} // namespace cycamore

