#ifndef CYCAMORE_SRC_TARIFF_REGION_TESTS_H_
#define CYCAMORE_SRC_TARIFF_REGION_TESTS_H_

#include <string>

#include <gtest/gtest.h>

#include "agent_tests.h"
#include "region_tests.h"
#include "tariff_region.h"
#include "test_context.h"

namespace cycamore {

class TariffRegionTests : public ::testing::Test {
 protected:
  virtual void SetUp();
  virtual void TearDown();

  void SetAdjustment(const std::string& region, const std::string& commodity,
                     double value, const std::string& type = "unit_cost");
  Adjustment FindAdjustment(cyclus::Region* supplier_region,
                            const std::string& commodity);
  void ValidateConfiguration();

  cyclus::TestContext tc_;
  TariffRegion* region_;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_TESTS_H_
