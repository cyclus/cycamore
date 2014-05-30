// source_tests.cc
#ifndef CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_
#define CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_

#include <gtest/gtest.h>

#include "source.h"

#include <boost/shared_ptr.hpp>

#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"
#include "agent_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SourceTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  cycamore::Source* src_facility;
  std::string commod, recipe_name;
  double capacity;
  cyclus::Composition::Ptr recipe;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSource();

  boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
      GetContext(int nreqs, std::string commodity);
};

#endif // CYCAMORE_MODELS_SOURCE_FACILITY_SOURCE_FACILITY_TESTS_
