#include <gtest/gtest.h>

#include "commodconverter_region.h"

#include "region_tests.h"
#include "agent_tests.h"

using commodconverter::CommodconverterRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CommodconverterRegionTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  CommodconverterRegion* src_region_;

  virtual void SetUp() {
    src_region_ = new CommodconverterRegion(tc_.get());
  }

  virtual void TearDown() {}
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterRegionTest, clone) {
  CommodconverterRegion* cloned_fac =
      dynamic_cast<CommodconverterRegion*> (src_region_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterRegionTest, Print) {
  EXPECT_NO_THROW(std::string s = src_region_->str());
  // Test CommodconverterRegion specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterRegionTest, Tick) {
  EXPECT_NO_THROW(src_region_->Tick());
  // Test CommodconverterRegion specific behaviors of the handleTick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodconverterRegionTest, Tock) {
  EXPECT_NO_THROW(src_region_->Tock());
  // Test CommodconverterRegion specific behaviors of the handleTock function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* CommodconverterRegionConstructor(cyclus::Context* ctx) {
  return new CommodconverterRegion(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(CommodconverterRegion, RegionTests,
                        ::testing::Values(&CommodconverterRegionConstructor));
INSTANTIATE_TEST_CASE_P(CommodconverterRegion, AgentTests,
                        ::testing::Values(&CommodconverterRegionConstructor));
