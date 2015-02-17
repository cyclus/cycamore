#ifndef CYCAMORE_SRC_FOO_TESTS_H_
#define CYCAMORE_SRC_FOO_TESTS_H_
#include "foo.h"

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "agent_tests.h"
#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FooTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  cycamore::Foo* src_facility;
  std::string commod, recipe_name;
  double capacity;
  cyclus::Composition::Ptr recipe;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpFoo();

  boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
      GetContext(int nreqs, std::string commodity);
};

#endif  // CYCAMORE_SRC_FOO_TESTS_H_
