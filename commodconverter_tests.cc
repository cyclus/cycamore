#include <gtest/gtest.h>

#include "commodconverter.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using commodconverter::CommodConverter;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CommodConverterTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  CommodConverter* src_facility_;

  virtual void SetUp() {
    src_facility_ = new CommodConverter(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, clone) {
  CommodConverter* cloned_fac =
      dynamic_cast<CommodConverter*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test CommodConverter specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test CommodConverter specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test CommodConverter specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* CommodConverterConstructor(cyclus::Context* ctx) {
  return new CommodConverter(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(CommodConverterFac, FacilityTests,
                        ::testing::Values(&CommodConverterConstructor));

INSTANTIATE_TEST_CASE_P(CommodConverterFac, AgentTests,
                        ::testing::Values(&CommodConverterConstructor));
