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

TEST_F(SourceTest, AddBids) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::ExchangeContext;
  using cyclus::Material;

  int nreqs = 5;

  boost::shared_ptr< cyclus::ExchangeContext<Material> >
      ec = GetContext(nreqs, commod);

  std::set<BidPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlBids(ec.get()->commod_requests);

  ASSERT_TRUE(ports.size() > 0);
  EXPECT_EQ(ports.size(), 1);

  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(port->bidder(), src_facility);
  EXPECT_EQ(port->bids().size(), nreqs);

  const std::set< CapacityConstraint<Material> >& constrs = port->constraints();
  ASSERT_TRUE(constrs.size() > 0);
  EXPECT_EQ(constrs.size(), 1);
  EXPECT_EQ(*constrs.begin(), CapacityConstraint<Material>(capacity));
}

TEST_F(SourceTest, Response) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::get_mat;

  std::vector< cyclus::Trade<cyclus::Material> > trades;
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> > responses;

  // Null response
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 0);

  double qty = capacity / 3;
  Request<Material>* request =
      Request<Material>::Create(get_mat(), trader, commod);
  Bid<Material>* bid =
      Bid<Material>::Create(request, get_mat(), src_facility);

  Trade<Material> trade(request, bid, qty);
  trades.push_back(trade);

  // 1 trade
  src_facility->GetMatlTrades(trades, responses);
  EXPECT_EQ(responses.size(), 1);
  EXPECT_EQ(responses[0].second->quantity(), qty);
  EXPECT_EQ(responses[0].second->comp(), recipe);

  // 2 trades, total qty = capacity
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);

  // reset!
  src_facility->Tick();

  delete request;
  delete bid;
}

TEST_F(SourceTest, CheckThrouputRecording) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
  ;
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Source"), config, simdur);
  sim.AddSink("spent_fuel").capacity(2).Finalize();
  int id = sim.Run();

  // checking the write amount of SWU has been repported
  cyclus::QueryResult qr = sim.db().Query("TimeSeriesThroughput", NULL);
  EXPECT_NEAR(qr.GetVal<double>("Value"), 2, 0.01);
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, StringMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config =
    "<outcommod>spent_fuel</outcommod>"
    " "
    " "
    "   <metadata>"
    "     <item> "
    "       <key>string_key</key>"
    "       <value>string_value%s</value>"
    "     </item> "
    "     <item> "
    "       <key>double_key</key>"
    "       <value>0.01254%d</value>"
    "     </item> "
    "     <item> "
    "       <key>int_key</key>"
    "       <value>-1254%i</value>"
    "     </item> "
    "     <item> "
    "       <key>uint_key</key>"
    "       <value>1254%u</value>"
    "     </item> "
    "     <item> "
    "       <key>bool_key</key>"
    "       <value>true%b</value>"
    "     </item> "
    "   </metadata>";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Source"), config, simdur);
  int id = sim.Run();

  std::vector<cyclus::Cond> conds;
  cyclus::QueryResult qr;
  conds.push_back(cyclus::Cond("keyword", "==", std::string("string_key")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "string_value");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "string");

  conds[0] = cyclus::Cond("keyword", "==", std::string("double_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "0.012540");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "double");

  conds[0] = cyclus::Cond("keyword", "==", std::string("int_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "-1254");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "int");

  conds[0] = cyclus::Cond("keyword", "==", std::string("uint_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "1254");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "uint");

  conds[0] = cyclus::Cond("keyword", "==", std::string("bool_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "true");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "bool");
}

TEST_F(SourceTest, UsageMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config =
    "   <outcommod>spent_fuel</outcommod>"
    "   "
    "   "
    "   <usagemetadata>"
    "     <item> "
    "       <keyword>co2</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>decommission</key> "
    "           <value>25</value> "
    "         </item> "
    "         <item> "
    "           <key>deployment</key> "
    "           <value>45</value> "
    "         </item> "
    "         <item> "
    "           <key>timestep</key> "
    "           <value>35</value> "
    "         </item> "
    "         <item> "
    "           <key>throughput</key> "
    "           <value>15</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>water</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>deployment</key> "
    "           <value>43</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>land</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>decommission</key> "
    "           <value>24</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>manpower</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>timestep</key> "
    "           <value>32</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>lolipop</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>throughput</key> "
    "           <value>11</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   </usagemetadata>";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Source"), config, simdur);
  int id = sim.Run();

  std::vector<cyclus::Cond> conds;
  cyclus::QueryResult qr;
  conds.push_back(cyclus::Cond("keyword", "==", std::string("co2")));
  conds.push_back(cyclus::Cond("Type", "==", std::string("decommission")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "25.000000");
  conds.clear();
  conds.push_back(cyclus::Cond("keyword", "==", std::string("co2")));
  conds.push_back(cyclus::Cond("Type", "==", std::string("deployment")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "45.000000");
  conds[1] = cyclus::Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "35.000000");
  conds[1] = cyclus::Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "15.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("water"));
  conds[1] = cyclus::Cond("Type", "==", std::string("deployment"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "43.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("land"));
  conds[1] = cyclus::Cond("Type", "==", std::string("decommission"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "24.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("manpower"));
  conds[1] = cyclus::Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "32.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("lolipop"));
  conds[1] = cyclus::Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "11.000000");
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

