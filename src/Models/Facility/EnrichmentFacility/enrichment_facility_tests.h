// enrichment_facility_tests.cc
#include <gtest/gtest.h>

#include "enrichment_facility.h"

#include "test_context.h"
#include "mock_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class EnrichmentFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  cycamore::EnrichmentFacility* src_facility;
  MockMarket* in_commod_market;
  MockMarket* out_commod_market;
  std::string in_commod, out_commod, in_recipe;
  cyclus::Composition::Ptr recipe;

  double tails_assay, feed_assay, inv_size, commodity_price;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void InitFacility();
};
