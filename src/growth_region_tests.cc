// growth_region_tests.cc

#include <sstream>

#include "growth_region_tests.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::SetUp() {
  ctx = new cyclus::Context(&ti, &rec);
  region = new cycamore::GrowthRegion(ctx);
  commodity_name = "commod";
  demand_type = "linear";
  demand_params = "5 5";
  demand_start = "0";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::TearDown() {
  delete region;
  delete ctx;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* GrowthRegionConstructor(cyclus::Context* ctx) {
  return new cycamore::GrowthRegion(ctx);
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool GrowthRegionTests::ManagesCommodity(cyclus::toolkit::Commodity& commodity) {
  return region->sdmanager()->ManagesCommodity(commodity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests, init) {
  cyclus::toolkit::Commodity commodity(commodity_name);
  cyclus::toolkit::ExpFunctionFactory eff;
  region->sdmanager()->RegisterCommodity(commodity, eff.GetFunctionPtr("2.0 4.0"));
  EXPECT_TRUE(ManagesCommodity(commodity));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionTests,
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, AgentTests,
                        Values(&GrowthRegionConstructor));

} // namespace cycamore

