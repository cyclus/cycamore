// enrichment_facility_tests.cc
#ifndef CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_TESTS_
#define CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_TESTS_

#include <gtest/gtest.h>

#include "test_context.h"
#include "enrichment_facility.h"
#include "material.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class EnrichmentFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  EnrichmentFacility* src_facility;
  std::string in_commod, out_commod, in_recipe;
  cyclus::Composition::Ptr recipe;

  double tails_assay, feed_assay, inv_size, commodity_price, swu_capacity;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSourceFacility();
  cyclus::Material::Ptr GetMat(double qty);
  void DoAbsorb(cyclus::Material::Ptr mat);
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_TESTS_
