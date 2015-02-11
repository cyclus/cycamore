#include <gtest/gtest.h>

#include <sstream>

#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::QueryResult;
using cyclus::Cond;

namespace cycamore {

TEST(ReactorTests, JustInTimeOrdering) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  Composition::Ptr fresh = Composition::CreateFromMass(m);
  m.clear();
  m[id("U235")] = .01;
  m[id("U238")] = .99;
  Composition::Ptr spent = Composition::CreateFromMass(m);

  std::string config = 
     "  <fuel_inrecipes>  <val>lwr_fresh</val>    </fuel_inrecipes>  "
     "  <fuel_outrecipes> <val>lwr_spent</val>    </fuel_outrecipes>  "
     "  <fuel_incommods>  <val>enriched_u</val>   </fuel_incommods>  "
     "  <fuel_outcommods> <val>waste</val>        </fuel_outcommods>  "
     "  <fuel_prefs>      <val>1.0</val>          </fuel_prefs>  "
     ""
     "  <cycle_time>1</cycle_time>  "
     "  <refuel_time>0</refuel_time>  "
     "  <assem_size>300</assem_size>  "
     "  <n_assem_core>1</n_assem_core>  "
     "  <n_assem_batch>1</n_assem_batch>  ";

  int simdur = 50;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Reactor"), config, simdur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddRecipe("lwr_fresh", fresh);
  sim.AddRecipe("lwr_spent", spent);
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(simdur, qr.rows.size()) << "failed to order+run on fresh fuel inside 1 time step";
}

}  // namespace cycamore

