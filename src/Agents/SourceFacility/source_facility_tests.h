// source_facility_tests.cc
#ifndef CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_
#define CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_

#include <gtest/gtest.h>

#include "source_facility.h"

#include <boost/shared_ptr.hpp>

#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"
#include "agent_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

#endif // CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_
