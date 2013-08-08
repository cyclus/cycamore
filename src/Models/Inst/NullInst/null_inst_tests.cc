// null_inst_tests.cc
#include <gtest/gtest.h>

#include "null_inst.h"

#include "inst_model_tests.h"
#include "model_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullInstTest : public ::testing::Test {
 protected:
  NullInst* src_inst;

  virtual void SetUp() {
    src_inst = new NullInst();
  }

  virtual void TearDown() {
    delete src_inst;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* NullInstModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new NullInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* NullInstConstructor() {
  return dynamic_cast<cyclus::InstModel*>(new NullInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullInst, InstModelTests, Values(&NullInstConstructor));
INSTANTIATE_TEST_CASE_P(NullInst, ModelTests,
                        Values(&NullInstModelConstructor));

