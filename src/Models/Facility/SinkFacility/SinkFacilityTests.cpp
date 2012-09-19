// SinkFacilityTests.cpp
#include <gtest/gtest.h>

#include "SinkFacility.h"
#include "CycException.h"
#include "Message.h"
#include "Model.h"
#include "FacilityModelTests.h"
#include "ModelTests.h"

#include <string>
#include <queue>

using namespace std;

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// class FakeSinkFacility : public SinkFacility {
//   public:
//     FakeSinkFacility() : SinkFacility() {
//       in_commods_.push_back("in-commod");
//       capacity_ = 2;
//       inventory_.setCapacity(50);
//       commod_price_ = 5000;
//     }

//     virtual ~FakeSinkFacility() {
//     }

//     double fakeCheckInventory() { return inventory_.quantity(); }

//     std::string getInCommod() {return in_commods_.front();}
//     double getCapacity() {return capacity_;}
//     double getInvSize() {return inventory_.capacity();}
//     double getCommodPrice() {return commod_price_;}
// };

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* SinkFacilityModelConstructor(){
  return dynamic_cast<Model*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FacilityModel* SinkFacilityConstructor(){
  return dynamic_cast<FacilityModel*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkFacilityTest : public ::testing::Test {
protected:
  SinkFacility* sink_facility;
  TestMarket* commod_market;
  std::string commod_;
    
  virtual void SetUp(){
    initParameters();
    setUpSinkFacility();
  }
  
  virtual void TearDown() {
    delete sink_facility;
    delete commod_market;
  }

  void initParameters() {
    commod_ = "incommod";
    commod_market = new TestMarket();
    commod_market->setCommodity(commod_);
    MarketModel::registerMarket(commod_market);
  }
  
  void setUpSinkFacility() {
    sink_facility = new SinkFacility();
    sink_facility->addCommodity(commod_);
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, sink_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest,clone) {
  SinkFacility* new_facility = dynamic_cast<SinkFacility*>(sink_facility->clone());
  EXPECT_EQ(sink_facility->capacity(),new_facility->capacity());
  EXPECT_EQ(sink_facility->maxInventorySize(),new_facility->maxInventorySize());
  delete new_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = sink_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, ReceiveMessage) {
  msg_ptr msg = msg_ptr(new Message(sink_facility));
  EXPECT_NO_THROW(sink_facility->receiveMessage(msg));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tick) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, sink_facility->inventorySize());
  EXPECT_NO_THROW(sink_facility->handleTick(time));
  EXPECT_DOUBLE_EQ(0.0,sink_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SinkFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0,sink_facility->inventorySize());
  EXPECT_NO_THROW(sink_facility->handleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityModelTests, Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, ModelTests, Values(&SinkFacilityModelConstructor));


