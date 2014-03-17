// deploy_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "deploy_inst.h"
#include "inst_model_tests.h"
#include "model_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* DeployInstitutionConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Agent*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Institution* DeployInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Institution*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
 protected:

  virtual void SetUp() {}

  virtual void TearDown() {}
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(DeployInst, InstitutionTests,
                        Values(&DeployInstConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, AgentTests,
                        Values(&DeployInstitutionConstructor));

