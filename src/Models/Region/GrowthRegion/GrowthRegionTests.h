// GrowthRegionTests.h
#ifndef GROWTHREGIONTESTS_H
#define GROWTHREGIONTESTS_H

#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "GrowthRegion.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class GrowthRegionTest : public ::testing::Test {
protected:
  GrowthRegion* reg_;
  GrowthRegion* new_region_; 
  virtual void SetUp();
  virtual void TearDown();
  int buildersSize(GrowthRegion* reg);
  xmlDocPtr getXMLDoc();
  void doInit(GrowthRegion* reg);
};

#endif
