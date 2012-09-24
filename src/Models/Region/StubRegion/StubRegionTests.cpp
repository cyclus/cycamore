// StubRegionTests.cpp
#include <gtest/gtest.h>

#include "StubRegion.h"

#include "Message.h"
#include "RegionModelTests.h"
#include "ModelTests.h"

#include <string>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StubRegionTest : public ::testing::Test {
  protected:
    StubRegion* src_region_;
    StubRegion* new_region_; 

    virtual void SetUp(){
      src_region_ = new StubRegion();
    };

    virtual void TearDown() {
      delete src_region_;
    }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* StubRegionModelConstructor(){
  return dynamic_cast<Model*>(new StubRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RegionModel* StubRegionConstructor(){
  return dynamic_cast<RegionModel*>(new StubRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubRegionTest, Print) {
  EXPECT_NO_THROW(std::string s = src_region_->str());
  // Test StubRegion specific aspects of the print method here
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubRegionTest, ReceiveMessage) {
  // Test StubRegion specific behaviors of the receiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(StubRegion, RegionModelTests, Values(&StubRegionConstructor));
INSTANTIATE_TEST_CASE_P(StubRegion, ModelTests, Values(&StubRegionModelConstructor));

