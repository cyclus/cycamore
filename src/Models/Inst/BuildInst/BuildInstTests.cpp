// BuildInstTests.cpp
#include <gtest/gtest.h>

#include "BuildInst.h"
#include "CycException.h"
#include "Message.h"
#include "InstModelTests.h"
#include "ModelTests.h"
#include "CycException.h"

#include <string>
#include <queue>


using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeBuildInst : public BuildInst {
public:
  FakeBuildInst() : BuildInst() {}
  
  virtual ~FakeBuildInst() {}

  void wrapAddPrototype(Model* prototype){
    addPrototype(prototype);
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BuildInstTest : public ::testing::Test {
protected:
  FakeBuildInst* src_inst;
  FakeBuildInst* new_inst; 
  TestRegion* tst_region;
  BuildInst* prototype;
  
  virtual void SetUp(){
    src_inst = new FakeBuildInst();
    tst_region = new TestRegion();
    src_inst->setParent(tst_region);
    new_inst = new FakeBuildInst();
    prototype = new BuildInst();
  };
  
  virtual void TearDown() {
    delete prototype;
    delete src_inst;
    delete new_inst;
    delete tst_region;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* BuildInstModelConstructor(){
  return dynamic_cast<Model*>(new FakeBuildInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
InstModel* BuildInstConstructor(){
  return dynamic_cast<InstModel*>(new FakeBuildInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, CopyFreshModel) {
  new_inst->copyFreshModel(dynamic_cast<Model*>(src_inst)); // deep copy
  EXPECT_NO_THROW(dynamic_cast<BuildInst*>(new_inst)); // still a build inst
  EXPECT_NO_THROW(dynamic_cast<FakeBuildInst*>(new_inst)); // still a fake build inst
  // Test that BuildInst specific parameters are initialized in the deep copy method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, Print) {
  EXPECT_NO_THROW(std::string s = src_inst->str());
  // Test BuildInst specific aspects of the print method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, ReceiveMessage) {
  msg_ptr msg;
  // Test BuildInst specific behaviors of the receiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_inst->handleTick(time));
  // Test BuildInst specific behaviors of the handleTick function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_inst->handleTick(time));
  // Test BuildInst specific behaviors of the handleTock function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(BuildInstTest, BuildPrototype) {
  EXPECT_NO_THROW(src_inst->wrapAddPrototype(prototype));
  EXPECT_TRUE(src_inst->isAvailablePrototype(prototype));
  EXPECT_THROW(src_inst->build(prototype,tst_region),CycIOException);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(BuildInst, InstModelTests, Values(&BuildInstConstructor));
INSTANTIATE_TEST_CASE_P(BuildInst, ModelTests, Values(&BuildInstModelConstructor));

