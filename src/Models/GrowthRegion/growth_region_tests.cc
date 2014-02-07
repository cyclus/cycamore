// growth_region_tests.cc
#include "growth_region_tests.h"

#include <sstream>

#include "region_model_tests.h"
#include "model_tests.h"
#include "commodity.h"
#include "xml_query_engine.h"
#include "xml_parser.h"

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
  delete ctx;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* GrowthRegionModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::GrowthRegion(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RegionModel* GrowthRegionConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::RegionModel*>(new cycamore::GrowthRegion(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::InitRegion() {
  using std::stringstream;
  stringstream ss("");
  ss << "<start>"
     << "<name>fooname</name>"
     << "<model>"
     << "<UNSPECIFIED>"
     << "  <commodity>"
     << "    <name>" << commodity_name << "</name>"
     << "    <demand>"
     << "      <type>" << demand_type << "</type>"
     << "      <parameters>" << demand_params << "</parameters>"
     << "      <start_time>" << demand_start << "</start_time>"
     << "    </demand>"
     << "  </commodity>"
     << "</UNSPECIFIED>"
     << "</model>"
     << "</start>";

  cyclus::XMLParser parser;
  parser.Init(ss);
  cyclus::XMLQueryEngine* engine = new cyclus::XMLQueryEngine(parser);
  region->InitFrom(engine);
  region->Deploy(NULL);
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool GrowthRegionTests::ManagesCommodity(cyclus::Commodity& commodity) {
  return region->sdmanager_.ManagesCommodity(commodity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests, init) {
  InitRegion();
  cyclus::Commodity commodity(commodity_name);
  EXPECT_TRUE(ManagesCommodity(commodity));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests,
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests,
                        Values(&GrowthRegionModelConstructor));

} // namespace cycamore

