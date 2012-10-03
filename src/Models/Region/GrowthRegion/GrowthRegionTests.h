// GrowthRegionTests.h
#ifndef GROWTHREGIONTESTS_H
#define GROWTHREGIONTESTS_H

#include <gtest/gtest.h>
#include <string>

#include "GrowthRegion.h"
#include "SupplyDemand.h"
#include "StubModel.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class GrowthRegionTest : public ::testing::Test {
protected:
  // region and constructor members
  GrowthRegion* reg_;
  GrowthRegion* new_region_; 

  // progeny interaction members
  StubModel* child1_;
  StubModel* child2_;

  // supply demand members
  int power_demand_;
  Commodity* commodity_;
  std::string commodity_name_;
  Producer* p1_;
  std::string producer1_name_;
  int producer1_cost_, producer2_cost_;
  Producer* p2_; 
  std::string producer2_name_;
  int producer1_capacity_, producer2_capacity_;

  // gtest construction and destruction
  virtual void SetUp();
  virtual void TearDown();
  
  // member introspection
  int containerSizes(GrowthRegion* reg);
  Producer getProducer(GrowthRegion* reg, int i);

  // member initialization
  std::string initString();
  void initSupplyDemand();
  void setUpChildren();
  void doInit();

  // method/member testing
  void testProducerInit();
  void testCommodityInit();
  void testProducerNames();
  void testMapsInit();
  void testBuildDecision();
};

#endif
