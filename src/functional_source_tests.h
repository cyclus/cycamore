#ifndef CYCAMORE_SRC_FUNCTIONAL_SOURCE_TESTS_H_
#define CYCAMORE_SRC_FUNCTIONAL_SOURCE_TESTS_H_
#include "functional_source.h"

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "agent_tests.h"
#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"

namespace cycamore {

class FunctionalSourceTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  cycamore::FunctionalSource* src_facility;
  std::string commod, recipe_name, package_name, tu_name;
  double capacity;
  cyclus::Composition::Ptr recipe;
  cyclus::Package::Ptr package;
  cyclus::TransportUnit::Ptr tu;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpFunctionalSource();

  std::string outrecipe(cycamore::FunctionalSource* s) { return s->outrecipe; }
  std::string outcommod(cycamore::FunctionalSource* s) { return s->outcommod; }
  double throughput(cycamore::FunctionalSource* s) { return s->throughput; }

  void outrecipe(cycamore::FunctionalSource* s, std::string recipe) {
    s->outrecipe = recipe;
  }
  void outcommod(cycamore::FunctionalSource* s, std::string commod) {
    s->outcommod = commod;
  }
  void throughput(cycamore::FunctionalSource* s, double val) { s->throughput = val; }
  void inventory_size(cycamore::FunctionalSource* s, double val) {
    s->inventory_size = val;
  }
  void incremental_production(cycamore::FunctionalSource* s, bool val) {
    s->incremental_production = val;
  }
  bool incremental_production(cycamore::FunctionalSource* s) {
    return s->incremental_production;
  }
  double inventory_quantity(cycamore::FunctionalSource* s) {
    return s->inventory.quantity();
  }
  void remove_inventory(cycamore::FunctionalSource* s, double qty) {
    s->inventory.Pop(qty);
  }

  boost::shared_ptr<cyclus::ExchangeContext<cyclus::Material> > GetContext(
      int nreqs, std::string commodity);
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_FUNCTIONAL_SOURCE_TESTS_H_
