#ifndef CYCAMORE_SRC_CONVERSION_TESTS_H_
#define CYCAMORE_SRC_CONVERSION_TESTS_H_

#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>

#include "agent_tests.h"
#include "context.h"
#include "exchange_context.h"
#include "facility_tests.h"
#include "material.h"
#include "test_context.h"
#include "conversion.h"

namespace cycamore {

class ConversionTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  cycamore::Conversion* conv_facility;
  std::string incommod1, incommod2, outcommod_name;
  double throughput_val, input_capacity_val;
  cyclus::Composition::Ptr recipe;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpConversion();

  // Accessor methods for private members
  std::vector<std::string> incommods(cycamore::Conversion* c) { return c->incommods; }
  std::string outcommod(cycamore::Conversion* c) { return c->outcommod; }
  double throughput(cycamore::Conversion* c) { return c->throughput; }
  double input_capacity(cycamore::Conversion* c) { return c->input_capacity; }

  // Setter methods for private members
  void incommods(cycamore::Conversion* c, std::vector<std::string> commods) {
    c->incommods = commods;
  }
  void outcommod(cycamore::Conversion* c, std::string commod) {
    c->outcommod = commod;
  }
  void throughput(cycamore::Conversion* c, double val) { c->throughput = val; }
  void input_capacity(cycamore::Conversion* c, double val) { c->input_capacity = val; }

  // Accessor methods for private buffers
  double input_quantity(cycamore::Conversion* c) { return c->input.quantity(); }
  double output_quantity(cycamore::Conversion* c) { return c->output.quantity(); }
  void input_push(cycamore::Conversion* c, cyclus::Material::Ptr mat) { c->input.Push(mat); }
  void output_push(cycamore::Conversion* c, cyclus::Material::Ptr mat) { c->output.Push(mat); }
  void set_input_capacity(cycamore::Conversion* c, double cap) { c->input.capacity(cap); }

  boost::shared_ptr<cyclus::ExchangeContext<cyclus::Material> > GetContext(
      int nreqs, std::string commodity);
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_CONVERSION_TESTS_H_ 