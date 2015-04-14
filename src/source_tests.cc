#include "source_tests.h"

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"

namespace cycamore {

void SourceTest::SetUp() {
  src_facility = new cycamore::Source(tc.get());
  trader = tc.trader();
  InitParameters();
  SetUpSource();
}

void SourceTest::TearDown() {
  delete src_facility;
}

void SourceTest::InitParameters() {
  commod = "commod";
  recipe_name = "recipe";
  capacity = 5;  // some magic number..

  recipe = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  tc.get()->AddRecipe(recipe_name, recipe);
}

void SourceTest::SetUpSource() {
  outcommod(src_facility, commod);
  outrecipe(src_facility, recipe_name);
  throughput(src_facility, capacity);
}

TEST_F(SourceTest, Clone) {
  cyclus::Context* ctx = tc.get();
  cycamore::Source* cloned_fac = dynamic_cast<cycamore::Source*>
                                         (src_facility->Clone());

  EXPECT_EQ(outcommod(src_facility),  outcommod(cloned_fac));
  EXPECT_EQ(throughput(src_facility), throughput(cloned_fac));
  EXPECT_EQ(outrecipe(src_facility),  outrecipe(cloned_fac));

  delete cloned_fac;
}

TEST_F(SourceTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

} // namespace cycamore

cyclus::Agent* SourceConstructor(cyclus::Context* ctx) {
  return new cycamore::Source(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

INSTANTIATE_TEST_CASE_P(SourceFac, FacilityTests, Values(&SourceConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, AgentTests, Values(&SourceConstructor));

