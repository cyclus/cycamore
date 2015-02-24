#include <gtest/gtest.h>

#include <sstream>

#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::Material;
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

Composition::Ptr c_natu() {
  cyclus::CompMap m;
  m[id("u235")] =  .007;
  m[id("u238")] =  .993;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustream() {
  cyclus::CompMap m;
  m[id("pu239")] = 100;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustreamlow() {
  cyclus::CompMap m;
  m[id("pu239")] = 80;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_pustreambad() {
  cyclus::CompMap m;
  m[id("pu239")] = 10;
  m[id("pu240")] = 10;
  m[id("pu241")] = 1;
  m[id("pu242")] = 1;
  return Composition::CreateFromMass(m);
};

Composition::Ptr c_water() {
  cyclus::CompMap m;
  m[id("O16")] =  1;
  m[id("H1")] =  2;
  return Composition::CreateFromAtom(m);
};

TEST(FuelFabTests, JustInTimeOrdering) {
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

} // namespace cycamore

