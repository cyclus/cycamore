#include <gtest/gtest.h>

#include "separationmatrix.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

using separationmatrix::SeparationMatrix;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationMatrixTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  SeparationMatrix* src_facility_;

  virtual void SetUp() {
    src_facility_ = new SeparationMatrix(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, clone) {
  SeparationMatrix* cloned_fac =
      dynamic_cast<SeparationMatrix*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, InitialState) {
  // Test things about the initial state of the facility here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test SeparationMatrix specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test SeparationMatrix specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test SeparationMatrix specific behaviors of the Tock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationMatrixConstructor(cyclus::Context* ctx) {
  return new SeparationMatrix(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationMatrixFac, FacilityTests,
                        ::testing::Values(&SeparationMatrixConstructor));

INSTANTIATE_TEST_CASE_P(SeparationMatrixFac, AgentTests,
                        ::testing::Values(&SeparationMatrixConstructor));
