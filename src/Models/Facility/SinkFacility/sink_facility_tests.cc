// sink_facility_tests.cc
#include <gtest/gtest.h>

#include "sink_facility_tests.h"

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
  commod_market->SetCommodity(commod_);
  cyclus::MarketModel::RegisterMarket(commod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::setUpSinkFacility() {
  using cycamore::SinkFacility;
  src_facility = new SinkFacility();
  src_facility->AddCommodity(commod_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, clone) {
  using cycamore::SinkFacility;
  SinkFacility* cloned_fac = new SinkFacility();
  cloned_fac->CloneModuleMembersFrom(src_facility);

  EXPECT_EQ(src_facility->capacity(), cloned_fac->capacity());
  EXPECT_EQ(src_facility->maxInventorySize(), cloned_fac->maxInventorySize());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, ReceiveMessage) {
  cyclus::Message::Ptr msg = cyclus::Message::Ptr(new cyclus::Message(
                                                    src_facility));
  EXPECT_NO_THROW(src_facility->ReceiveMessage(msg));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Tick) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
  EXPECT_NO_THROW(src_facility->HandleTick(time));
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
  EXPECT_NO_THROW(src_facility->HandleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityModelTests,
                        Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, ModelTests,
                        Values(&SinkFacilityModelConstructor));


