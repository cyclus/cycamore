#include <gtest/gtest.h>

#include <string>

#include "commodconverter_inst.h"

#include "institution_tests.h"
#include "agent_tests.h"

using commodconverter::CommodconverterInst;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CommodconverterInstTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  CommodconverterInst* src_inst_;

  virtual void SetUp() {
    src_inst_ = new CommodconverterInst(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterInstTest, clone) {
  CommodconverterInst* cloned_fac =
      dynamic_cast<CommodconverterInst*> (src_inst_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterInstTest, Print) {
  EXPECT_NO_THROW(std::string s = src_inst_->str());
  // Test CommodconverterInst specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterInstTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test CommodconverterInst specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterInstTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_inst_->Tick());
  // Test CommodconverterInst specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* CommodconverterInstitutionConstructor(cyclus::Context* ctx) {
  return new CommodconverterInst(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(CommodconverterInst, InstitutionTests,
                        ::testing::Values(&CommodconverterInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(CommodconverterInst, AgentTests,
                        ::testing::Values(&CommodconverterInstitutionConstructor));
