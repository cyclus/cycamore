// stub_inst_tests.cc
#include <gtest/gtest.h>

#include "stub_inst.h"
#include "message.h"
#include "inst_model_tests.h"
#include "model_tests.h"

#include <string>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StubInstTest : public ::testing::Test {
 protected:
  StubInst* src_inst;

  virtual void SetUp() {
    src_inst = new StubInst();
  };

  virtual void TearDown() {
    delete src_inst;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* StubInstModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new StubInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* StubInstConstructor() {
  return dynamic_cast<cyclus::InstModel*>(new StubInst());
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, Print) {
  EXPECT_NO_THROW(string s = src_inst->str());
  // Test StubInst specific aspects of the print method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, ReceiveMessage) {
  cyclus::Message::Ptr msg;
  // Test StubInst specific behaviors of the ReceiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_inst->HandleTick(time));
  // Test StubInst specific behaviors of the HandleTick function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_inst->HandleTick(time));
  // Test StubInst specific behaviors of the HandleTock function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StubInst, InstModelTests, Values(&StubInstConstructor));
INSTANTIATE_TEST_CASE_P(StubInst, ModelTests,
                        Values(&StubInstModelConstructor));

