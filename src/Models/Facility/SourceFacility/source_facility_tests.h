// source_facility_tests.cc
#include <gtest/gtest.h>

#include "source_facility.h"

#include "context.h"
#include "facility_model_tests.h"
#include "model_tests.h"
#include "test_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::SourceFacility;
  return dynamic_cast<cyclus::Model*>(new SourceFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SourceFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::SourceFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new SourceFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  cycamore::SourceFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_, recipe_name_;
  cyclus::Composition::Ptr recipe_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSourceFacility();
};
