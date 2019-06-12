#include <sstream>

#include "growth_region_tests.h"
#if CYCLUS_HAS_COIN

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::SetUp() {
  ctx = new cyclus::Context(&ti, &rec);
  region = new cycamore::GrowthRegion(ctx);
  commodity_name = "commod";
  demand_type = "linear";
  demand_params = "5 5";
  demand_start = "0";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::TearDown() {
  delete region;
  delete ctx;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool GrowthRegionTests::ManagesCommodity(cyclus::toolkit::Commodity& commodity) {
  return region->sdmanager()->ManagesCommodity(commodity);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests, init) {
  cyclus::toolkit::Commodity commodity(commodity_name);
  cyclus::toolkit::ExpFunctionFactory eff;
  region->sdmanager()->RegisterCommodity(commodity, eff.GetFunctionPtr("2.0 4.0"));
  EXPECT_TRUE(ManagesCommodity(commodity));
}

TEST_F(GrowthRegionTests, PositionDefault) {
  std::string config = 
     "<prototypes>  <val>foobar</val> </prototypes>"
     "<growth>"
        "<commod>test_commod</commod>"
          "<piecewise_function>"
          "<piece>"
            "<start>0</start>"
            "<function>"
              "<type>linear</type>"
              "<params>1 2</params>"
            "</function>"
          "</piece>"
        "</piecewise_function>"
      "</growth>";

  int simdur = 5;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:GrowthRegion"), config, simdur);
  sim.DummyProto("foobar");
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);
}

TEST_F(GrowthRegionTests, PositionInitialize) {
  std::string config = 
     "<prototypes>  <val>foobar</val> </prototypes>"
     "<longitude>   -20.0             </longitude>"
     "<latitude>    2.0               </latitude>"
     "<growth>"
        "<commod>test_commod</commod>"
          "<piecewise_function>"
          "<piece>"
            "<start>0</start>"
            "<function>"
              "<type>linear</type>"
              "<params>1 2</params>"
            "</function>"
          "</piece>"
        "</piecewise_function>"
      "</growth>";

  int simdur = 5;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:GrowthRegion"), config, simdur);
  sim.DummyProto("foobar");
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 2.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), -20.0);
}

}  // namespace cycamore

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* GrowthRegionConstructor(cyclus::Context* ctx) {
  return new cycamore::GrowthRegion(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionTests,
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, AgentTests,
                        Values(&GrowthRegionConstructor));
#endif  // CYCLUS_HAS_COIN
