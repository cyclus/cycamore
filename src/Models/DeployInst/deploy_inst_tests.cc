// deploy_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "deploy_inst.h"
#include "inst_model_tests.h"
#include "model_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* DeployInstModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* DeployInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstModel*>(new cycamore::DeployInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
 protected:

  virtual void SetUp() {}

  virtual void TearDown() {}
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(DeployInst, InstModelTests,
                        Values(&DeployInstConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, ModelTests,
                        Values(&DeployInstModelConstructor));

