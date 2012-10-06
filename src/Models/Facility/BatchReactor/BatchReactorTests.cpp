// BatchReactorTests.cpp
#include <gtest/gtest.h>

#include "BatchReactorTests.h"
#include "Commodity.h"
#include "XMLQueryEngine.h"
#include "Model.h"
#include <sstream>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactorTest::SetUp() 
{
  // set up model parameters
  lencycle = 3;
  loadcore = 10.0;
  nbatch = 5;
  in_commod = "inc";
  out_commod = "outc";
  in_recipe = "inr";
  out_recipe = "outr";
  commodity = "power";
  capacity = 200;
  cost = capacity;
  initSrcFacility();
  initWorld();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactorTest::TearDown() 
{
  delete src_facility;
  delete incommod_market;
  delete outcommod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactorTest::initSrcFacility() 
{
  stringstream ss("");
  ss << "<start>"
     << "  <fuel_input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "  </fuel_input>"
     << "  <fuel_output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <outrecipe>" << out_recipe << "</outrecipe>"
     << "  </fuel_output>"
     << "  <cyclelength>" << lencycle << "</cyclelength>"
     << "  <coreloading>" << loadcore << "</coreloading>"
     << "  <batchespercore>" << nbatch << "</batchespercore>"
     << "  <commodity_production>"
     << "    <commodity>" << commodity << "</commodity>"
     << "    <capacity>" << capacity << "</capacity>"
     << "    <cost>" << cost << "</cost>"
     << "  </commodity_production>"
     << "</start>";

  XMLParser parser(ss);
  XMLQueryEngine* engine = new XMLQueryEngine(parser);
  src_facility = new BatchReactor();
  src_facility->initModuleMembers(engine);
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactorTest::initWorld() 
{
  incommod_market = new TestMarket();
  incommod_market->setCommodity(in_commod);
  MarketModel::registerMarket(incommod_market);

  outcommod_market = new TestMarket();
  outcommod_market->setCommodity(out_commod);
  MarketModel::registerMarket(outcommod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BatchReactorTest,initialstate) 
{
  EXPECT_EQ(lencycle,src_facility->cycleLength());
  EXPECT_EQ(loadcore,src_facility->coreLoading());
  EXPECT_EQ(nbatch,src_facility->nBatches());
  Commodity commod(commodity);
  EXPECT_TRUE(src_facility->producesCommodity(commod)); 
  EXPECT_EQ(capacity,src_facility->productionCapacity(commod));
  EXPECT_EQ(cost,src_facility->productionCost(commod));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BatchReactorTest,clone) 
{
  Model::loadModule("Facility","BatchReactor");
  src_facility->setModelImpl("BatchReactor");
  BatchReactor* new_facility = dynamic_cast<BatchReactor*>(src_facility->clone());
  EXPECT_EQ( lencycle, new_facility->cycleLength() );
  EXPECT_EQ( loadcore, new_facility->coreLoading() );
  EXPECT_EQ( nbatch, new_facility->nBatches() );
  Commodity commod(commodity);
  EXPECT_TRUE(new_facility->producesCommodity(commod)); 
  EXPECT_EQ(capacity,new_facility->productionCapacity(commod));
  EXPECT_EQ(cost,new_facility->productionCost(commod));

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BatchReactorTest, Print) 
{
  EXPECT_NO_THROW(std::string s = src_facility->str());
  //Test BatchReactor specific aspects of the print method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BatchReactorTest, Tick) 
{
  int time = 1;
  EXPECT_NO_THROW(src_facility->handleTick(time););
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BatchReactorTest, Tock) 
{
  int time = 1;
  EXPECT_ANY_THROW(src_facility->handleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(BatchReactor, FacilityModelTests, Values(&BatchReactorConstructor));
INSTANTIATE_TEST_CASE_P(BatchReactor, ModelTests, Values(&BatchReactorModelConstructor));

