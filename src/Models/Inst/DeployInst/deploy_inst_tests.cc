// deploy_inst_tests.cc
#include <gtest/gtest.h>

#include "deploy_inst.h"
#include "inst_model_tests.h"
#include "model_tests.h"



//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* DeployInstModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new DeployInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* DeployInstConstructor() {
  return dynamic_cast<cyclus::InstModel*>(new DeployInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
 protected:
  DeployInst* src_inst;

  virtual void SetUp() {
    src_inst = new DeployInst();
  }

  virtual void TearDown() {
    delete src_inst;
  }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(DeployInst, InstModelTests,
                        Values(&DeployInstConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, ModelTests,
                        Values(&DeployInstModelConstructor));

