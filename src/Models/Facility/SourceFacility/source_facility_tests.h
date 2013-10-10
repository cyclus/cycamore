// source_facility_tests.cc
#include <gtest/gtest.h>

#include "source_facility.h"

#include "context.h"
#include "facility_model_tests.h"
#include "model_tests.h"
#include "mock_market.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  cycamore::SourceFacility* src_facility;
  MockMarket* commod_market;
  std::string commod_, recipe_name_;
  cyclus::Composition::Ptr recipe_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSourceFacility();
};
