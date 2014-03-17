// null_region_tests.cc
#include <gtest/gtest.h>

#include "null_region.h"
#include "error.h"
#include "region_tests.h"
#include "agent_tests.h"
#include "timer.h"
#include "recorder.h"

#include <string>
#include <queue>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeNullRegion : public cycamore::NullRegion {
 public:
  FakeNullRegion(cyclus::Context* ctx)
      : cycamore::NullRegion(ctx) {}

  virtual ~FakeNullRegion() {}
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullRegionTest : public ::testing::Test {
 protected:

  virtual void SetUp() {};

  virtual void TearDown() {};
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* NullRegionConstructor(cyclus::Context* ctx) {
  return new FakeNullRegion(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(NullRegionTest, InitialState) {
  // Test things about the initial state of the region here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(NullRegionTest, Print) {
  cyclus::Timer ti;
  cyclus::Recorder rec;
  cyclus::Context ctx(&ti, &rec);
  FakeNullRegion reg(&ctx);
  EXPECT_NO_THROW(std::string s = reg.str());
  // Test NullRegion specific aspects of the print method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullRegion, RegionTests,
                        Values(&NullRegionConstructor));
INSTANTIATE_TEST_CASE_P(NullRegion, AgentTests,
                        Values(&NullRegionConstructor));


