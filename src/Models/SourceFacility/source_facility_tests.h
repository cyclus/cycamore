// source_facility_tests.cc
#ifndef CYCAMORE_SRC_MODELS_SOURCEFACILITY_SOURCE_FACILITY_TESTS_H_
#define CYCAMORE_SRC_MODELS_SOURCEFACILITY_SOURCE_FACILITY_TESTS_H_

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "context.h"
#include "exchange_context.h"
#include "facility_model_tests.h"
#include "material.h"
#include "model_tests.h"
#include "source_facility.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceFacilityTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  cycamore::SourceFacility* src_facility;
  std::string commod, recipe_name;
  double capacity;
  cyclus::Composition::Ptr recipe;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSourceFacility();

  boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
      GetContext(int nreqs, std::string commodity);
};

#endif  // CYCAMORE_SRC_MODELS_SOURCEFACILITY_SOURCE_FACILITY_TESTS_H_
