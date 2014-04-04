// deploy_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "deploy_inst.h"
#include "institution_tests.h"
#include "agent_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* DeployInstitutionConstructor(cyclus::Context* ctx) {
  return new cycamore::DeployInst(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
 protected:

  virtual void SetUp() {}

  virtual void TearDown() {}
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(DeployInst, InstitutionTests,
                        Values(&DeployInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, AgentTests,
                        Values(&DeployInstitutionConstructor));

