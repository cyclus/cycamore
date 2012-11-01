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
  in_loadcore = 10.0;
  out_loadcore = 9.0;
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
     << "  <incoreloading>" << in_loadcore << "</incoreloading>"
     << "  <outcoreloading>" << out_loadcore << "</outcoreloading>"
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
  EXPECT_EQ(lencycle,src_facility->cycle_length());
  EXPECT_EQ(in_loadcore,src_facility->in_core_loading());
  EXPECT_EQ(out_loadcore,src_facility->out_core_loading());
  EXPECT_EQ(nbatch,src_facility->batches_per_core());
  EXPECT_EQ(in_commod,src_facility->in_commodity());
  EXPECT_EQ(out_commod,src_facility->out_commodity());
  EXPECT_EQ(in_recipe,src_facility->in_recipe());
  EXPECT_EQ(out_recipe,src_facility->out_recipe());
  
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
  EXPECT_EQ( lencycle, new_facility->cycle_length() );
  EXPECT_EQ( in_loadcore, new_facility->in_core_loading() );
  EXPECT_EQ( out_loadcore, new_facility->out_core_loading() );
  EXPECT_EQ( nbatch, new_facility->batches_per_core() );
  EXPECT_EQ(in_commod,new_facility->in_commodity());
  EXPECT_EQ(out_commod,new_facility->out_commodity());
  EXPECT_EQ(in_recipe,new_facility->in_recipe());
  EXPECT_EQ(out_recipe,new_facility->out_recipe());

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

