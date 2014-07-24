#include <gtest/gtest.h>

#include "commodconverter_facility.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using commodconverter::CommodconverterFacility;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CommodconverterFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  CommodconverterFacility* src_facility_;

  virtual void SetUp() {
    src_facility_ = new CommodconverterFacility(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterFacilityTest, clone) {
  CommodconverterFacility* cloned_fac =
      dynamic_cast<CommodconverterFacility*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterFacilityTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test CommodconverterFacility specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterFacilityTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test CommodconverterFacility specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterFacilityTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test CommodconverterFacility specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* CommodconverterFacilityConstructor(cyclus::Context* ctx) {
  return new CommodconverterFacility(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(CommodconverterFac, FacilityTests,
                        ::testing::Values(&CommodconverterFacilityConstructor));

INSTANTIATE_TEST_CASE_P(CommodconverterFac, AgentTests,
                        ::testing::Values(&CommodconverterFacilityConstructor));
