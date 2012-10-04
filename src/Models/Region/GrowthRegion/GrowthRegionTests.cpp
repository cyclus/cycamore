// GrowthRegionTests.cpp
#include "GrowthRegionTests.h"
#include "RegionModelTests.h"
#include "ModelTests.h"

#include "Commodity.h"
#include "XMLQueryEngine.h"

#include <sstream>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTests::SetUp() 
{
  region = new GrowthRegion();
  commodity_name = "commod";
  demand_type = "linear";
  demand_params = "5 5";
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTests::TearDown() 
{
  delete region;
}  

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* GrowthRegionModelConstructor()
{
  return dynamic_cast<Model*>(new GrowthRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RegionModel* GrowthRegionConstructor()
{
  return dynamic_cast<RegionModel*>(new GrowthRegion());
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
     << "    </demand>"
     << "  </commodity>"
     << "</start>";

  XMLParser parser(ss);
  XMLQueryEngine* engine = new XMLQueryEngine(parser);
  region->initModuleMembers(engine);
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool GrowthRegionTests::managesCommodity(Commodity& commodity)
{
  return region->sdmanager_.managesCommodity(commodity); 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests,init) 
{
  initRegion();
  Commodity commodity(commodity_name);
  EXPECT_TRUE(managesCommodity(commodity));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

