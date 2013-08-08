// null_region_tests.cc
#include <gtest/gtest.h>

#include "null_region.h"
#include "error.h"
#include "message.h"
#include "region_model_tests.h"
#include "model_tests.h"

#include <string>
#include <queue>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeNullRegion : public cycamore::NullRegion {
 public:
  FakeNullRegion() : cycamore::NullRegion() { }

  virtual ~FakeNullRegion() { }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullRegionTest : public ::testing::Test {
 protected:
  FakeNullRegion* src_region_;
  FakeNullRegion* new_region_;

  virtual void SetUp() {
    src_region_ = new FakeNullRegion();
    new_region_ = new FakeNullRegion();
  };

  virtual void TearDown() {
    delete src_region_;
    delete new_region_;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* NullRegionModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new FakeNullRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RegionModel* NullRegionConstructor() {
  return dynamic_cast<cyclus::RegionModel*>(new FakeNullRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(NullRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(NullRegionTest, Print) {
  EXPECT_NO_THROW(std::string s = src_region_->str());
  // Test NullRegion specific aspects of the print method here
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(NullRegionTest, ReceiveMessage) {
  // Test NullRegion specific behaviors of the ReceiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullRegion, RegionModelTests,
                        Values(&NullRegionConstructor));
INSTANTIATE_TEST_CASE_P(NullRegion, ModelTests,
                        Values(&NullRegionModelConstructor));


