// EnrichmentFacilityTests.cpp
#include <gtest/gtest.h>

#include "EnrichmentFacilityTests.h"

#include "FacilityModelTests.h"
#include "ModelTests.h"

#include "Commodity.h"
#include "XMLQueryEngine.h"

#include <sstream>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::SetUp() 
{
  src_facility = new EnrichmentFacility();

  initParameters();
  initFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::TearDown() 
{
  delete src_facility;
  delete out_commod_market;
  delete in_commod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::initParameters() 
{
  in_commod = "incommod";
  in_commod_market = new TestMarket();
  in_commod_market->setCommodity(in_commod);
  MarketModel::registerMarket(in_commod_market);

  out_commod = "outcommod";
  out_commod_market = new TestMarket();
  out_commod_market->setCommodity(out_commod);
  MarketModel::registerMarket(out_commod_market);

  in_recipe = "recipe";
  feed_assay = 0.0072;
  recipe = CompMapPtr(new CompMap(ATOM));
  (*recipe)[92235] = feed_assay;
  (*recipe)[92238] = 1-feed_assay;
  RecipeLibrary::recordRecipe(in_recipe,recipe);

  tails_assay = 0.002;
  inv_size = 5;
  commodity_price = 0;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::initFacility() 
{
  stringstream ss("");
  ss << "<start>"
     << "  <input>"
     <<	"    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "    <inventorysize>" << inv_size << "</inventorysize>"
     << "  </input>"
     << "  <output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <tails_assay>" << tails_assay << "</tails_assay>"
     << "  </output>"
     << "</start>";

  XMLParser parser(ss);
  XMLQueryEngine* engine = new XMLQueryEngine(parser);

  EXPECT_NO_THROW(src_facility->initModuleMembers(engine));
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest,init) 
{
  EXPECT_EQ(in_recipe,src_facility->in_recipe());
  EXPECT_EQ(in_commod,src_facility->in_commodity());
  EXPECT_EQ(out_commod,src_facility->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay,src_facility->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay,src_facility->feed_assay());  
  EXPECT_DOUBLE_EQ(inv_size,src_facility->maxInventorySize());  
  EXPECT_DOUBLE_EQ(commodity_price,src_facility->commodity_price());  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(EnrichmentFacilityTest,clone) 
{
  EnrichmentFacility* cloned_fac = new EnrichmentFacility();
  cloned_fac->cloneModuleMembersFrom(src_facility);
 
  EXPECT_EQ(in_recipe,cloned_fac->in_recipe());
  EXPECT_EQ(in_commod,cloned_fac->in_commodity());
  EXPECT_EQ(out_commod,cloned_fac->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay,cloned_fac->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay,cloned_fac->feed_assay());
  EXPECT_DOUBLE_EQ(inv_size,cloned_fac->maxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price,cloned_fac->commodity_price());  
  
  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Model* EnrichmentFacilityModelConstructor()
{
  return dynamic_cast<Model*>(new EnrichmentFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
FacilityModel* EnrichmentFacilityConstructor()
{
  return dynamic_cast<FacilityModel*>(new EnrichmentFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(EnrichmentFac, FacilityModelTests, Values(&EnrichmentFacilityConstructor));
INSTANTIATE_TEST_CASE_P(EnrichmentFac, ModelTests, Values(&EnrichmentFacilityModelConstructor));
