#include <gtest/gtest.h>

#include "separationmatrix_facility.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using separationmatrix::SeparationmatrixFacility;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationmatrixFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  SeparationmatrixFacility* src_facility_;

  virtual void SetUp() {
    src_facility_ = new SeparationmatrixFacility(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixFacilityTest, clone) {
  SeparationmatrixFacility* cloned_fac =
      dynamic_cast<SeparationmatrixFacility*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixFacilityTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test SeparationmatrixFacility specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixFacilityTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test SeparationmatrixFacility specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixFacilityTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test SeparationmatrixFacility specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationmatrixFacilityConstructor(cyclus::Context* ctx) {
  return new SeparationmatrixFacility(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationmatrixFac, FacilityTests,
                        ::testing::Values(&SeparationmatrixFacilityConstructor));

INSTANTIATE_TEST_CASE_P(SeparationmatrixFac, AgentTests,
                        ::testing::Values(&SeparationmatrixFacilityConstructor));
