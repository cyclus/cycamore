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
  virtual void SetUp() { };

  virtual void TearDown() { };
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* StubRegionModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::StubRegion(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RegionModel* StubRegionConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::RegionModel*>(new cycamore::StubRegion(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StubRegionTest, InitialState) {
  // Test things about the initial state of the region here
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

