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
  delete src_facility;
  delete commod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::initParameters() {
  commod_ = "incommod";
  commod_market = new TestMarket();
  commod_market->setcyclus::Commodity(commod_);
  MarketModel::registerMarket(commod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacilityTest::setUpSinkFacility() {
  src_facility = new SinkFacility();
  src_facility->addCommodity(commod_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest,clone) {
  SinkFacility* cloned_fac = new SinkFacility();
  cloned_fac->cloneModuleMembersFrom(src_facility);
  
  EXPECT_EQ(src_facility->capacity(),cloned_fac->capacity());
  EXPECT_EQ(src_facility->maxInventorySize(),cloned_fac->maxInventorySize());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, ReceiveMessage) {
  msg_ptr msg = msg_ptr(new Message(src_facility));
  EXPECT_NO_THROW(src_facility->receiveMessage(msg));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tick) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
  EXPECT_NO_THROW(src_facility->handleTick(time));
  EXPECT_DOUBLE_EQ(0.0,src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0,src_facility->inventorySize());
  EXPECT_NO_THROW(src_facility->handleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityModelTests, Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, ModelTests, Values(&SinkFacilityModelConstructor));


