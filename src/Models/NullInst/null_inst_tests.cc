// null_inst_tests.cc
#include <gtest/gtest.h>

#include "null_inst.h"

#include "context.h"
#include "inst_model_tests.h"
#include "model_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullInstTest : public ::testing::Test {
 protected:
  virtual void SetUp() {}

  virtual void TearDown() {}
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* NullInstModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::NullInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* NullInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstModel*>(new cycamore::NullInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullInst, InstModelTests, Values(&NullInstConstructor));
INSTANTIATE_TEST_CASE_P(NullInst, ModelTests,
                        Values(&NullInstModelConstructor));

