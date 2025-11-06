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
#include "pyne.h"

namespace cycamore {

class ConversionTest : public ::testing::Test {
 public:
  cyclus::TestContext tc;
  TestFacility* trader;
  cycamore::Conversion* conv_facility;
  std::string incommod1, outcommod_name;
  double throughput_val, input_capacity_val;
  cyclus::Composition::Ptr recipe;

  const double TEST_QUANTITY = 10.0;  

  const int SIMULATION_DURATION = 3;
  const double DEFAULT_POSITION = 0.0;
  const double TEST_POSITION = 0.01;
  
  const double DEFAULT_THROUGHPUT = 10.0;
  const double DEFAULT_INPUT_CAPACITY = 50.0;
  
  const std::string DEFAULT_CONFIG = 
    "<incommods>"
    "  <incommodity>incommod1</incommodity>"
    "</incommods>"
    "<outcommod>outcommod</outcommod>"
    "<throughput>" + std::to_string(DEFAULT_THROUGHPUT) + "</throughput>"
    "<input_capacity>" + std::to_string(DEFAULT_INPUT_CAPACITY) + "</input_capacity>";

  const std::string INCOMMOD1 = "incommod1";
  const std::string OUTCOMMOD_NAME = "outcommod";

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

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_CONVERSION_TESTS_H_ 