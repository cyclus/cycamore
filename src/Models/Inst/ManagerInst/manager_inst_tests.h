// manager_inst_tests.h
#include <gtest/gtest.h>

#include "context.h"
#include "commodity_producer.h"
#include "event_manager.h"
#include "facility_model.h"
#include "manager_inst.h"
#include "timer.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TestProducer :
  public cyclus::FacilityModel,
  public cyclus::supply_demand::CommodityProducer {
 public:
  TestProducer(cyclus::Context* ctx);
  virtual ~TestProducer();

  virtual cyclus::Model* Clone() {
    TestProducer* m = new TestProducer(*this);
    m->initfrom(this);
    return m;
  }

  void CloneModuleMembersFrom(cyclus::FacilityModel* source) {};
  void HandleTock(int time) {};
  void HandleTick(int time) {};

  void ReceiveMessage(cyclus::Message::Ptr msg) {
    msg->SetDir(cyclus::DOWN_MSG);
  }

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
  cyclus::EventManager em_;

 public:
  virtual void SetUp();
  virtual void TearDown();
};
