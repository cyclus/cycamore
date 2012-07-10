// GrowthRegionTests.h
#ifndef GROWTHREGIONTESTS_H
#define GROWTHREGIONTESTS_H

#include <gtest/gtest.h>

#include "GrowthRegion.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class GrowthRegionTest : public ::testing::Test {
protected:
  GrowthRegion* reg_;
  GrowthRegion* new_region_; 
  virtual void SetUp();
  virtual void TearDown();
};

#endif
