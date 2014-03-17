// null_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "inst_model_tests.h"
#include "model_tests.h"
#include "null_inst.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullInstTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* NullInstAgentConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Agent*>(new cycamore::NullInst(ctx));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstAgent* NullInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstAgent*>(new cycamore::NullInst(ctx));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullInst, InstAgentTests,
                        Values(&NullInstConstructor));
INSTANTIATE_TEST_CASE_P(NullInst, AgentTests,
                        Values(&NullInstAgentConstructor));

