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

  void handleTick(int time) {}
  void handleTock(int time) {}
  void cloneModuleMembersFrom(FacilityModel*) {}

  void receiveMessage(msg_ptr msg) {
    msg->setDir(DOWN_MSG);
  }

  /* void receiveMaterial(Transaction trans, std::vector<mat_rsrc_ptr> manifest) { } */
  
  /* Prototype* clone() { return new TestFacility(); } */

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
