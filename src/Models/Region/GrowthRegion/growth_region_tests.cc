// growth_region_tests.cc
#include "growth_region_tests.h"
#include "RegionModelTests.h"
#include "ModelTests.h"

#include "Commodity.h"
#include "XMLQueryEngine.h"
#include "XMLParser.h"

#include <sstream>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTests::SetUp() 
{
  region = new GrowthRegion();
  commodity_name = "commod";
  demand_type = "linear";
  demand_params = "5 5";
  demand_start = "0";
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTests::TearDown() 
{
  delete region;
}  

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* GrowthRegionModelConstructor()
{
  return dynamic_cast<cyclus::Model*>(new GrowthRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RegionModel* GrowthRegionConstructor()
{
  return dynamic_cast<cyclus::RegionModel*>(new GrowthRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTests::initRegion() 
{
  stringstream ss("");
  ss << "<start>"
     << "  <commodity>"
     << "    <name>" << commodity_name << "</name>"
     << "    <demand>"
     << "      <type>" << demand_type << "</type>"
     << "      <parameters>" << demand_params << "</parameters>"
     << "      <start_time>" << demand_start << "</start_time>"
     << "    </demand>"
     << "  </commodity>"
     << "</start>";

  cyclus::XMLParser parser;
  parser.init(ss);
  cyclus::XMLQueryEngine* engine = new cyclus::XMLQueryEngine(parser);
  region->initModuleMembers(engine);
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool GrowthRegionTests::managesCommodity(cyclus::Commodity& commodity)
{
  return region->sdmanager_.managesCommodity(commodity); 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests,init) 
{
  initRegion();
  cyclus::Commodity commodity(commodity_name);
  EXPECT_TRUE(managesCommodity(commodity));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

