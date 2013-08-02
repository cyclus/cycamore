// GrowthRegionTests.h
#ifndef GROWTHREGIONTESTS_H
#define GROWTHREGIONTESTS_H

#include <gtest/gtest.h>

#include "GrowthRegion.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class GrowthRegionTests : public ::testing::Test {
 protected:
  GrowthRegion* region;
  std::string commodity_name, demand_type, demand_params, demand_start;

  virtual void SetUp();
  virtual void TearDown();
  void initRegion();
  bool managesCommodity(cyclus::Commodity& commodity);
};

#endif
