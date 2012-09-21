// SinkFacilityTests.cpp
#include <gtest/gtest.h>

#include "SinkFacilityTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::SetUp() {
  initParameters();
  setUpSinkFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::TearDown() {
  delete sink_facility;
  delete commod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::initParameters() {
  commod_ = "incommod";
  commod_market = new TestMarket();
  commod_market->setCommodity(commod_);
  MarketModel::registerMarket(commod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::setUpSinkFacility() {
  sink_facility = new SinkFacility();
  sink_facility->addCommodity(commod_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, sink_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest,clone) {
  SinkFacility* new_facility = dynamic_cast<SinkFacility*>(sink_facility->clone());
  EXPECT_EQ(sink_facility->capacity(),new_facility->capacity());
  EXPECT_EQ(sink_facility->maxInventorySize(),new_facility->maxInventorySize());
  delete new_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = sink_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, ReceiveMessage) {
  msg_ptr msg = msg_ptr(new Message(sink_facility));
  EXPECT_NO_THROW(sink_facility->receiveMessage(msg));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tick) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, sink_facility->inventorySize());
  EXPECT_NO_THROW(sink_facility->handleTick(time));
  EXPECT_DOUBLE_EQ(0.0,sink_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0,sink_facility->inventorySize());
  EXPECT_NO_THROW(sink_facility->handleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityModelTests, Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, ModelTests, Values(&SinkFacilityModelConstructor));


