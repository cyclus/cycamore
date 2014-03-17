// deploy_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "deploy_inst.h"
#include "inst_model_tests.h"
#include "model_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* DeployInstAgentConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Agent*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstAgent* DeployInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstAgent*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
 protected:

  virtual void SetUp() {}

  virtual void TearDown() {}
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(DeployInst, InstAgentTests,
                        Values(&DeployInstConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, AgentTests,
                        Values(&DeployInstAgentConstructor));

