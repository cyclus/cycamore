#include <gtest/gtest.h>

#include "separationmatrix_region.h"

#include "region_tests.h"
#include "agent_tests.h"

using separationmatrix::SeparationmatrixRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationmatrixRegionTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  SeparationmatrixRegion* src_region_;

  virtual void SetUp() {
    src_region_ = new SeparationmatrixRegion(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixRegionTest, clone) {
  SeparationmatrixRegion* cloned_fac =
      dynamic_cast<SeparationmatrixRegion*> (src_region_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixRegionTest, Print) {
  EXPECT_NO_THROW(std::string s = src_region_->str());
  // Test SeparationmatrixRegion specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixRegionTest, Tick) {
  EXPECT_NO_THROW(src_region_->Tick());
  // Test SeparationmatrixRegion specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationmatrixRegionTest, Tock) {
  EXPECT_NO_THROW(src_region_->Tock());
  // Test SeparationmatrixRegion specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationmatrixRegionConstructor(cyclus::Context* ctx) {
  return new SeparationmatrixRegion(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationmatrixRegion, RegionTests,
                        ::testing::Values(&SeparationmatrixRegionConstructor));
INSTANTIATE_TEST_CASE_P(SeparationmatrixRegion, AgentTests,
                        ::testing::Values(&SeparationmatrixRegionConstructor));
