#include <gtest/gtest.h>

#include <string>

#include "separationmatrix_inst.h"

#include "institution_tests.h"
#include "agent_tests.h"

using separationmatrix::SeparationmatrixInst;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationmatrixInstTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  SeparationmatrixInst* src_inst_;

  virtual void SetUp() {
    src_inst_ = new SeparationmatrixInst(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixInstTest, clone) {
  SeparationmatrixInst* cloned_fac =
      dynamic_cast<SeparationmatrixInst*> (src_inst_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixInstTest, Print) {
  EXPECT_NO_THROW(std::string s = src_inst_->str());
  // Test SeparationmatrixInst specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixInstTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test SeparationmatrixInst specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixInstTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test SeparationmatrixInst specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationmatrixInstitutionConstructor(cyclus::Context* ctx) {
  return new SeparationmatrixInst(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationmatrixInst, InstitutionTests,
                        ::testing::Values(&SeparationmatrixInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(SeparationmatrixInst, AgentTests,
                        ::testing::Values(&SeparationmatrixInstitutionConstructor));
