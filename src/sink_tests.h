// sink_tests.cc
#include <gtest/gtest.h>

#include "sink.h"

#include "test_context.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  TestFacility* trader;
  cycamore::Sink* src_facility;
  std::string commod1_, commod2_, commod3_;
  double capacity_, inv_, qty_;
  int ncommods_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSink();
};
