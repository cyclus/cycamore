#include <gtest/gtest.h>

#include "context.h"
#include "group_inst.h"
#include "institution_tests.h"
#include "agent_tests.h"


TEST(GroupInstTests, BuildTimes) {
  std::string config = 
       " <prototypes>"
       "   <val>foobar1</val>"
       "   <val>foobar2</val>"
       "   <val>foobar3</val>"
       " </prototypes>"
     ;

  int simdur = 5;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:GroupInst"), config, simdur);
  sim.DummyProto("foobar1");
  sim.DummyProto("foobar2");
  sim.DummyProto("foobar3");
  int id = sim.Run();

  cyclus::SqlStatement::Ptr stmt = sim.db().db().Prepare(
      "SELECT COUNT(*) FROM AgentEntry WHERE Prototype = 'foobar1';"
      );
  stmt->Step();
  EXPECT_EQ(1, stmt->GetInt(0));

  stmt = sim.db().db().Prepare(
      "SELECT COUNT(*) FROM AgentEntry WHERE Prototype = 'foobar2';"
      );
  stmt->Step();
  EXPECT_EQ(1, stmt->GetInt(0));
  stmt = sim.db().db().Prepare(
      "SELECT COUNT(*) FROM AgentEntry WHERE Prototype = 'foobar3';"
      );
  stmt->Step();
  EXPECT_EQ(1, stmt->GetInt(0));
}

// required to get functionality in cyclus agent unit tests library
cyclus::Agent* GroupInstitutionConstructor(cyclus::Context* ctx) {
  return new cycamore::GroupInst(ctx);
}
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

INSTANTIATE_TEST_CASE_P(GroupInst, InstitutionTests,
                        Values(&GroupInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(GroupInst, AgentTests,
                        Values(&GroupInstitutionConstructor));
