// sink_facility_tests.cc
#ifndef CYCAMORE_SRC_MODELS_SINKFACILITY_SINK_FACILITY_TESTS_H_
#define CYCAMORE_SRC_MODELS_SINKFACILITY_SINK_FACILITY_TESTS_H_

#include <gtest/gtest.h>

#include "sink_facility.h"
#include "test_context.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkFacilityTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  cycamore::SinkFacility* src_facility;
  std::string commod1_, commod2_, commod3_;
  double capacity_, inv_, qty_;
  int ncommods_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSinkFacility();
};
#endif  // CYCAMORE_SRC_MODELS_SINKFACILITY_SINK_FACILITY_TESTS_H_
