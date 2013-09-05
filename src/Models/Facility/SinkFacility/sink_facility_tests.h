// sink_facility_tests.cc
#include <gtest/gtest.h>

#include "sink_facility.h"

#include "facility_model_tests.h"
#include "model_tests.h"
#include "test_context.h"
#include "test_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SinkFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::Model*>(new SinkFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SinkFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new SinkFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  cycamore::SinkFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSinkFacility();
};
