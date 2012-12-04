// ManagerInstTests.h
#include <gtest/gtest.h>

#include "ManagerInst.h"
#include "FacilityModel.h"
#include "CommodityProducer.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class TestProducer : 
  public FacilityModel, 
  public SupplyDemand::CommodityProducer
{
 public:
  TestProducer();
  virtual ~TestProducer();

  void cloneModuleMembersFrom(FacilityModel* source) {};
  void handleTock(int time){};
  void handleTick(int time){};

  void receiveMessage(msg_ptr msg) {
    msg->setDir(DOWN_MSG);
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class ManagerInstTests : public ::testing::Test 
{
 protected:
  ManagerInst* src_inst;
  TestProducer* producer;
  
  Commodity commodity;
  double capacity;

 public:  
  virtual void SetUp();
  virtual void TearDown();
};
