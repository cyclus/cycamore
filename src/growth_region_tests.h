#ifndef CYCAMORE_SRC_GROWTH_REGION_TESTS_H_
#define CYCAMORE_SRC_GROWTH_REGION_TESTS_H_

#include <gtest/gtest.h>

#include "agent_tests.h"
#include "context.h"
#include "recorder.h"
#include "region_tests.h"
#include "test_context.h"
#include "timer.h"

#include "growth_region.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class GrowthRegionTests : public ::testing::Test {
 protected:
  cyclus::Context* ctx;
  cyclus::Timer ti;
  cyclus::Recorder rec;
  cycamore::GrowthRegion* region;
  std::string commodity_name, demand_type, demand_params, demand_start;

  virtual void SetUp();
  virtual void TearDown();
  bool ManagesCommodity(cyclus::toolkit::Commodity& commodity);
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_GROWTH_REGION_TESTS_H_
