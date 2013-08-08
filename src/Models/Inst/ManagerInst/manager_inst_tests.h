// manager_inst_tests.h
#include <gtest/gtest.h>

#include "manager_inst.h"
#include "facility_model.h"
#include "commodity_producer.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TestProducer :
  public cyclus::FacilityModel,
  public cyclus::supply_demand::CommodityProducer {
 public:
  TestProducer();
  virtual ~TestProducer();

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

 public:
  virtual void SetUp();
  virtual void TearDown();
};
