// GrowthRegionTests.h
#ifndef GROWTHREGIONTESTS_H
#define GROWTHREGIONTESTS_H

#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string>

#include "GrowthRegion.h"
#include "SupplyDemand.h"
#include "StubModel.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class GrowthRegionTest : public ::testing::Test {
protected:
  GrowthRegion* reg_;
  GrowthRegion* new_region_; 
  virtual void SetUp();
  virtual void TearDown();
  int containerSizes(GrowthRegion* reg);
  xmlDocPtr getXMLDoc();
  Producer getProducer(GrowthRegion* reg, int i);
  int producer1_capacity_, producer2_capacity_;
  int producer1_cost_, producer2_cost_;
  std::string producer1_name_, producer2_name_;
  std::string commodity_name_;
  void initProducers();
  StubModel* child1_;
  StubModel* child2_;
  Commodity* commodity_;
  Producer* p1_;
  Producer* p2_;
  void testProducerInit();
  void testCommodityInit();
  void testMapsInit();
  void setUpChildren();
  void testProducerNames();
  void testBuildDecision();
  void doInit();
};

#endif
