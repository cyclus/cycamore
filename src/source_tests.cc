#include "source_tests.h"

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"

using cyclus::QueryResult;

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

TEST_F(SourceTest, Trade) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
    "<throughput>100</throughput>"
  ;
  int simdur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Source"), config, simdur);
  sim.AddSink("spent_fuel").Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(simdur, qr.rows.size());
}

TEST_F(SourceTest, Buffer) {
  std::string config = 
    "<outcommod>spent_fuel</outcommod>"
    "<buffer>1</buffer>"
    "<throughput>100</throughput>"
    "<inventory_size>1e6</inventory_size>"
  ;
  int simdur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Source"), config, simdur);
  sim.AddSink("spent_fuel")
     .start(5)
     .Finalize();
  int id = sim.Run();

  cyclus::SqlStatement::Ptr stmt = sim.db().db().Prepare(
      "SELECT COUNT(*) FROM transactions INNER JOIN resources"
      " ON resources.resourceid = transactions.resourceid"
      " WHERE quantity == 100");
  stmt->Step();
  EXPECT_EQ(4, stmt->GetInt(0));

  stmt = sim.db().db().Prepare(
      "SELECT COUNT(*) FROM transactions INNER JOIN resources"
      " ON resources.resourceid = transactions.resourceid"
      " WHERE quantity == 600");
  stmt->Step();
  EXPECT_EQ(1, stmt->GetInt(0));

}


TEST_F(SourceTest, PositionInitialize) {
  std::string config = 
    "<outcommod>spent_fuel</outcommod>"
  ;
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Source"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);

}

TEST_F(SourceTest, Longitude) {
  std::string config = 
    "<outcommod>spent_fuel</outcommod>"
    "<latitude>-0.01</latitude>"
    "<longitude>0.01</longitude>"
  ;
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Source"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), -0.01);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.01);

}

boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
SourceTest::GetContext(int nreqs, std::string commod) {
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ExchangeContext;
  using test_helpers::get_mat;

  double qty = 3;
  boost::shared_ptr< ExchangeContext<Material> >
      ec(new ExchangeContext<Material>());
  for (int i = 0; i < nreqs; i++) {
    ec->AddRequest(Request<Material>::Create(get_mat(), trader, commod));
  }
  return ec;
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

