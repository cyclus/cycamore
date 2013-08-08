// stub_region_tests.cc
#include <gtest/gtest.h>

#include "stub_region.h"

#include "message.h"
#include "region_model_tests.h"
#include "model_tests.h"

#include <string>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StubRegionTest : public ::testing::Test {
 protected:
  StubRegion* src_region_;
  StubRegion* new_region_;

  virtual void SetUp() {
    src_region_ = new StubRegion();
  };

  virtual void TearDown() {
    delete src_region_;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* StubRegionModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new StubRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RegionModel* StubRegionConstructor() {
  return dynamic_cast<cyclus::RegionModel*>(new StubRegion());
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
  // Test StubRegion specific behaviors of the ReceiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StubRegion, RegionModelTests,
                        Values(&StubRegionConstructor));
INSTANTIATE_TEST_CASE_P(StubRegion, ModelTests,
                        Values(&StubRegionModelConstructor));

