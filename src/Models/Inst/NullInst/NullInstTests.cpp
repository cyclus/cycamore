// NullInstTests.cpp
#include <gtest/gtest.h>

#include "NullInst.h"

#include "InstModelTests.h"
#include "ModelTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullInstTest : public ::testing::Test {
protected:
  NullInst* src_inst;
  
  virtual void SetUp(){
    src_inst = new NullInst();
  }
  
  virtual void TearDown() {
    delete src_inst;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* NullInstModelConstructor(){
  return dynamic_cast<Model*>(new NullInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
InstModel* NullInstConstructor(){
  return dynamic_cast<InstModel*>(new NullInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(NullInst, InstModelTests, Values(&NullInstConstructor));
INSTANTIATE_TEST_CASE_P(NullInst, ModelTests, Values(&NullInstModelConstructor));

