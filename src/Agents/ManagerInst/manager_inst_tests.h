// manager_inst_tests.h
#include <gtest/gtest.h>

#include "context.h"
#include "commodity_producer.h"
#include "recorder.h"
#include "facility.h"
#include "manager_inst.h"
#include "timer.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TestProducer :
  public cyclus::Facility,
  public cyclus::CommodityProducer {
 public:
  TestProducer(cyclus::Context* ctx);
  virtual ~TestProducer();

  virtual cyclus::Agent* Clone() {
    TestProducer* m = new TestProducer(context());
    m->InitFrom(this);
    return m;
  };

  void InitFrom(TestProducer* m) {
    cyclus::Facility::InitFrom(m);
  };

  void Tock(int time) {};
  void Tick(int time) {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class ManagerInstTests : public ::testing::Test {
 protected:
  cycamore::ManagerInst* src_inst;
  TestProducer* producer;

  cyclus::Commodity commodity;
  double capacity;
  cyclus::Context* ctx_;
  cyclus::Timer ti_;
  cyclus::Recorder rec_;

 public:
  virtual void SetUp();
  virtual void TearDown();
};
