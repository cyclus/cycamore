// EnrichmentFacilityTests.cpp
#include <gtest/gtest.h>

#include "EnrichmentFacility.h"

#include "TestMarket.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class EnrichmentFacilityTest : public ::testing::Test {
protected:
  EnrichmentFacility* src_facility;
  TestMarket* in_commod_market;
  TestMarket* out_commod_market;
  std::string in_commod, out_commod, in_recipe;
  cyclus::CompMapPtr recipe;
  
  double tails_assay, feed_assay, inv_size, commodity_price;
    
  virtual void SetUp();
  virtual void TearDown();
  void initParameters();
  void initFacility();
};
