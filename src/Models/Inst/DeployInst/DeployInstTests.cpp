// DeployInstTests.cpp
#include <gtest/gtest.h>

#include "DeployInst.h"
#include "InstModelTests.h"
#include "ModelTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* DeployInstModelConstructor(){
  return dynamic_cast<Model*>(new DeployInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
InstModel* DeployInstConstructor(){
  return dynamic_cast<InstModel*>(new DeployInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class DeployInstTest : public ::testing::Test {
  protected:
    DeployInst* src_inst;

    virtual void SetUp(){
      src_inst = new DeployInst();
     }

    virtual void TearDown() {
      delete src_inst;
    }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(DeployInst, InstModelTests, Values(&DeployInstConstructor));
INSTANTIATE_TEST_CASE_P(DeployInst, ModelTests, Values(&DeployInstModelConstructor));

