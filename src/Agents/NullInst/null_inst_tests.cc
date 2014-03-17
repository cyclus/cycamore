// null_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "institution_tests.h"
#include "agent_tests.h"
#include "null_inst.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullInstTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* NullInstitutionConstructor(cyclus::Context* ctx) {
  return new cycamore::NullInst(ctx);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullInst, InstitutionTests,
                        Values(&NullInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(NullInst, AgentTests,
                        Values(&NullInstitutionConstructor));

