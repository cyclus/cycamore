// sink_facility_tests.cc
#include <gtest/gtest.h>

#include "sink_facility.h"

#include "facility_model_tests.h"
#include "model_tests.h"
#include "test_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SinkFacilityModelConstructor() {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::Model*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SinkFacilityConstructor() {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkFacilityTest : public ::testing::Test {
 protected:
  cycamore::SinkFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_;

  virtual void SetUp();
  virtual void TearDown();
  void initParameters();
  void setUpSinkFacility();
};
