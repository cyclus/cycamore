// inpro_reactor_tests.cc

#include "inpro_reactor_tests.h"

#include <sstream>

#include <gtest/gtest.h>

// #include "commodity.h"
#include "facility_tests.h"
#include "agent_tests.h"
#include "agent.h"
#include "xml_infile_tree.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactorTest::SetUp() {
  // set up agent parameters
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
  src_facility = new cycamore::InproReactor(tc_.get());
  InitSrcFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactorTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactorTest::InitSrcFacility() {
  std::stringstream ss("");
  ss << "<start>"
     << "<name>fooname</name>"
     << "<agent>"
     << "<UNSPECIFIED>"
     << "  <fuel_input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "  </fuel_input>"
     << "  <fuel_output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <outrecipe>" << out_recipe << "</outrecipe>"
     << "  </fuel_output>"
     << "  <cyclelength>" << lencycle << "</cyclelength>"
     << "  <refueldelay>" << lencycle << "</refueldelay>"
     << "  <incoreloading>" << in_loadcore << "</incoreloading>"
     << "  <outcoreloading>" << out_loadcore << "</outcoreloading>"
     << "  <batchespercore>" << nbatch << "</batchespercore>"
     << "  <commodity_production>"
     << "    <commodity>" << commodity << "</commodity>"
     << "    <capacity>" << capacity << "</capacity>"
     << "    <cost>" << cost << "</cost>"
     << "  </commodity_production>"
     << "</UNSPECIFIED>"
     << "</agent>"
     << "</start>";

  cyclus::XMLParser parser;
  parser.Init(ss);
  cyclus::XMLInfileTree* engine = new cyclus::XMLInfileTree(parser);
  src_facility->InitFrom(engine);
  delete engine;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(InproReactorTest, initialstate) {
  EXPECT_EQ(lencycle, src_facility->cycle_length());
  EXPECT_EQ(lencycle, src_facility->refuel_delay());
  EXPECT_EQ(in_loadcore, src_facility->in_core_loading());
  EXPECT_EQ(out_loadcore, src_facility->out_core_loading());
  EXPECT_EQ(nbatch, src_facility->batches_per_core());
  EXPECT_EQ(in_commod, src_facility->in_commodity());
  EXPECT_EQ(out_commod, src_facility->out_commodity());
  EXPECT_EQ(in_recipe, src_facility->in_recipe());
  EXPECT_EQ(out_recipe, src_facility->out_recipe());

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(src_facility->ProducesCommodity(commod));
  EXPECT_EQ(capacity, src_facility->ProductionCapacity(commod));
  EXPECT_EQ(cost, src_facility->ProductionCost(commod));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(InproReactorTest, clone) {
  using cycamore::InproReactor;
  InproReactor* cloned_fac = dynamic_cast<cycamore::InproReactor*>
                             (src_facility->Clone());

  EXPECT_EQ(lencycle, cloned_fac->cycle_length());
  EXPECT_EQ(lencycle, cloned_fac->refuel_delay());
  EXPECT_EQ(in_loadcore, cloned_fac->in_core_loading());
  EXPECT_EQ(out_loadcore, cloned_fac->out_core_loading());
  EXPECT_EQ(nbatch, cloned_fac->batches_per_core());
  EXPECT_EQ(in_commod, cloned_fac->in_commodity());
  EXPECT_EQ(out_commod, cloned_fac->out_commodity());
  EXPECT_EQ(in_recipe, cloned_fac->in_recipe());
  EXPECT_EQ(out_recipe, cloned_fac->out_recipe());

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(cloned_fac->ProducesCommodity(commod));
  EXPECT_EQ(capacity, cloned_fac->ProductionCapacity(commod));
  EXPECT_EQ(cost, cloned_fac->ProductionCost(commod));

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(InproReactorTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
  //Test InproReactor specific aspects of the print method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(InproReactorTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_facility->Tick(time););
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(InproReactorTest, Tock) {
  int time = 1;
  EXPECT_ANY_THROW(src_facility->Tock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* InproReactorAgentConstructor(cyclus::Context* ctx) {
  using cycamore::InproReactor;
  return dynamic_cast<cyclus::Agent*>(new InproReactor(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Facility* InproReactorConstructor(cyclus::Context* ctx) {
  using cycamore::InproReactor;
  return dynamic_cast<cyclus::Facility*>(new InproReactor(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(InproReactor, FacilityTests,
                        Values(&InproReactorConstructor));
INSTANTIATE_TEST_CASE_P(InproReactor, AgentTests,
                        Values(&InproReactorAgentConstructor));

