// ManagerInstTests.h
#include <gtest/gtest.h>

#include "ManagerInst.h"
#include "FacilityModel.h"
#include "CommodityProducer.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class TestProducer : 
  public cyclus::FacilityModel, 
  public cyclus::SupplyDemand::CommodityProducer
{
 public:
  TestProducer();
  virtual ~TestProducer();

  void cloneModuleMembersFrom(cyclus::FacilityModel* source) {};
  void handleTock(int time){};
  void handleTick(int time){};

  void receiveMessage(cyclus::msg_ptr msg) {
    msg->setDir(cyclus::DOWN_MSG);
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class ManagerInstTests : public ::testing::Test 
{
 protected:
  ManagerInst* src_inst;
  TestProducer* producer;
  
  cyclus::Commodity commodity;
  double capacity;

 public:  
  virtual void SetUp();
  virtual void TearDown();
};
