// enrichment_facility_tests.cc
#include <gtest/gtest.h>

#include "enrichment_facility.h"

#include "test_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class EnrichmentFacilityTest : public ::testing::Test {
 protected:
  cycamore::EnrichmentFacility* src_facility;
  TestMarket* in_commod_market;
  TestMarket* out_commod_market;
  std::string in_commod, out_commod, in_recipe;
  cyclus::Composition::Ptr recipe;

  double tails_assay, feed_assay, inv_size, commodity_price;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void InitFacility();
};
