// source_facility_tests.cc
#include <gtest/gtest.h>

#include "source_facility.h"

#include "facility_model_tests.h"
#include "model_tests.h"
#include "test_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacilityModelConstructor() {
  return dynamic_cast<cyclus::Model*>(new SourceFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SourceFacilityConstructor() {
  return dynamic_cast<cyclus::FacilityModel*>(new SourceFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceFacilityTest : public ::testing::Test {
 protected:
  SourceFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_, recipe_name_;
  cyclus::CompMapPtr recipe_;

  virtual void SetUp();
  virtual void TearDown();
  void initParameters();
  void setUpSourceFacility();
};
