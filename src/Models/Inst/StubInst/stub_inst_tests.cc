// stub_inst_tests.cc
#include <gtest/gtest.h>

#include "context.h"
#include "event_manager.h"
#include "stub_inst.h"
#include "message.h"
#include "inst_model_tests.h"
#include "model_tests.h"
#include "timer.h"

#include <string>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StubInstTest : public ::testing::Test {
 protected:
  cycamore::StubInst* src_inst;
  cyclus::Context* ctx_;
  cyclus::Timer ti_;
  cyclus::EventManager em_;

  virtual void SetUp() {
    ctx_ = new cyclus::Context(&ti_, &em_);
    src_inst = new cycamore::StubInst(ctx_);
  };

  virtual void TearDown() {
    delete src_inst;
    delete ctx_;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* StubInstModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::StubInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* StubInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstModel*>(new cycamore::StubInst(ctx));
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, InitialState) {
  // Test things about the initial state of the inst here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubInstTest, Print) {
  EXPECT_NO_THROW(std::string s = src_inst->str());
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

