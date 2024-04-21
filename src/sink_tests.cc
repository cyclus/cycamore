#include <gtest/gtest.h>

#include "facility_tests.h"
#include "agent_tests.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "xml_parser.h"

#include "sink_tests.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::SetUp() {
  src_facility = new cycamore::Sink(tc_.get());
  trader = tc_.trader();
  InitParameters();
  SetUpSink();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::TearDown() {
  delete src_facility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::InitParameters() {
  commod1_ = "acommod";
  commod2_ = "bcommod";
  commod3_ = "ccommod";
  capacity_ = 5;
  inv_ = capacity_ * 2;
  qty_ = capacity_ * 0.5;
  ncommods_ = 2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::SetUpSink() {
  src_facility->AddCommodity(commod1_);
  src_facility->AddCommodity(commod2_);
  src_facility->Capacity(capacity_);
  src_facility->SetMaxInventorySize(inv_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, InitialState) {
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->Capacity());
  EXPECT_DOUBLE_EQ(inv_, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->SpaceAvailable());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, src_facility->input_commodities());

  src_facility->EnterNotify();
  double pref[] = {cyclus::kDefaultPref, cyclus::kDefaultPref};
  std::vector<double> vpref (pref, pref + sizeof(pref) / sizeof(pref[0]) );
  EXPECT_EQ(vpref, src_facility->input_commodity_preferences());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Clone) {
  using cycamore::Sink;
  Sink* cloned_fac = dynamic_cast<cycamore::Sink*>
                             (src_facility->Clone());

  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->Capacity());
  EXPECT_DOUBLE_EQ(inv_, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->SpaceAvailable());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, cloned_fac->input_commodities());

  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<config>"
     << "<UNSPECIFIED>"
     << "<input>"
     << "  <commodities>"
     << "  <incommodity>" << commod1_ << "</incommodity>"
     << "  <incommodity>" << commod2_ << "</incommodity>"
     << "  </commodities>"
     << "  <input_capacity>" << capacity_ << "</input_capacity>"
     << "  <inventorysize>" << inv_ << "</inventorysize>"
     << "</input>"
     << "</UNSPECIFIED>"
     << "</config>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::Sink fac(tc_.get());

  // EXPECT_NO_THROW(fac.InitFrom(&engine););
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, fac.input_commodities());
  EXPECT_DOUBLE_EQ(capacity_, fac.Capacity());
  EXPECT_DOUBLE_EQ(inv_, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, fac.SpaceAvailable());
  EXPECT_DOUBLE_EQ(0.0, fac.InventorySize());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Requests) {
  using cyclus::Request;
  using cyclus::RequestPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;

  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> commods (arr, arr + sizeof(arr) / sizeof(arr[0]) );

  src_facility->EnterNotify();
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();

  ASSERT_EQ(ports.size(), 1);
  ASSERT_EQ(ports.begin()->get()->qty(), capacity_);
  const std::vector<Request<Material>*>& requests =
      ports.begin()->get()->requests();
  ASSERT_EQ(requests.size(), 2);

  for (int i = 0; i < ncommods_; ++i) {
    Request<Material>* req = *(requests.begin() + i);
    EXPECT_EQ(req->requester(), src_facility);
    EXPECT_EQ(req->commodity(), commods[i]);
  }

  const std::set< CapacityConstraint<Material> >& constraints =
      ports.begin()->get()->constraints();
  EXPECT_EQ(constraints.size(), 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, EmptyRequests) {
  using cyclus::Material;
  using cyclus::RequestPortfolio;

  src_facility->Capacity(0);
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();
  EXPECT_TRUE(ports.empty());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Accept) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::get_mat;

  double qty = qty_ * 2;
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> > responses;

  Request<Material>* req1 =
      Request<Material>::Create(get_mat(922350000, qty_), src_facility,
                                commod1_);
  Bid<Material>* bid1 = Bid<Material>::Create(req1, get_mat(), trader);

  Request<Material>* req2 =
      Request<Material>::Create(get_mat(922350000, qty_), src_facility,
                                commod2_);
  Bid<Material>* bid2 =
      Bid<Material>::Create(req2, get_mat(922350000, qty_), trader);

  Trade<Material> trade1(req1, bid1, qty_);
  responses.push_back(std::make_pair(trade1, get_mat(922350000, qty_)));
  Trade<Material> trade2(req2, bid2, qty_);
  responses.push_back(std::make_pair(trade2, get_mat(922350000, qty_)));

  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  src_facility->AcceptMatlTrades(responses);
  EXPECT_DOUBLE_EQ(qty, src_facility->InventorySize());
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, InRecipe){
// Create a context
  using cyclus::RequestPortfolio;
  using cyclus::Material;
  using cyclus::Request;
  cyclus::Recorder rec;
  cyclus::Timer ti;
  cyclus::Context ctx(&ti, &rec);

  // define some test material in the context
  cyclus::CompMap m;
  m[922350000] = 1;
  m[922580000] = 2;

  cyclus::Composition::Ptr c = cyclus::Composition::CreateFromMass(m);
  ctx.AddRecipe("some_u",c) ;

  // create a sink facility to interact with the DRE
  cycamore::Sink* snk = new cycamore::Sink(&ctx);
  snk->AddCommodity("some_u");
  snk->EnterNotify();

  std::set<RequestPortfolio<Material>::Ptr> ports =
    snk->GetMatlRequests();
  ASSERT_EQ(ports.size(), 1);

  const std::vector<Request<Material>*>& requests =
    ports.begin()->get()->requests();
  ASSERT_EQ(requests.size(), 1);

  Request<Material>* req = *requests.begin();
  EXPECT_EQ(req->requester(), snk);
  EXPECT_EQ(req->commodity(),"some_u");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, BidPrefs) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "     <val>commods_2</val>"
    "   </in_commods>"
    "   <in_commod_prefs>"
    "     <val>10</val> "
    "     <val>1</val> "
    "   </in_commod_prefs>"
    "   <capacity>1</capacity>"
    "   <input_capacity>1.0</input_capacity> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);

  sim.AddSource("commods_1")
    .capacity(1)
    .Finalize();

  sim.AddSource("commods_2")
    .capacity(1)
    .Finalize();

  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==", std::string("commods_1")));
  QueryResult qr = sim.db().Query("Transactions", &conds);

  // should trade only with #1 since it has highier priority
  EXPECT_EQ(1, qr.rows.size());

  std::vector<Cond> conds2;
  conds2.push_back(Cond("Commodity", "==", std::string("commods_2")));
  QueryResult qr2 = sim.db().Query("Transactions", &conds2);

  // should trade only with #1 since it has highier priority
  EXPECT_EQ(0, qr2.rows.size());

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

TEST_F(SinkTest, PositionInitialize) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "     <val>commods_2</val>"
    "   </in_commods>"
    "   <in_commod_prefs>"
    "     <val>10</val> "
    "     <val>1</val> "
    "   </in_commod_prefs>"
    "   <capacity>1</capacity>"
    "   <input_capacity>1.0</input_capacity> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);

  sim.AddSource("commods_1")
    .capacity(1)
    .Finalize();

  sim.AddSource("commods_2")
    .capacity(1)
    .Finalize();

  int id = sim.Run();

  QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);
}

TEST_F(SinkTest, PositionInitialize2) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "     <val>commods_2</val>"
    "   </in_commods>"
    "   <in_commod_prefs>"
    "     <val>10</val> "
    "     <val>1</val> "
    "   </in_commod_prefs>"
    "   <capacity>1</capacity>"
    "   <input_capacity>1.0</input_capacity> "
    "   <latitude>50.0</latitude> "
    "   <longitude>35.0</longitude> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);

  sim.AddSource("commods_1")
    .capacity(1)
    .Finalize();

  sim.AddSource("commods_2")
    .capacity(1)
    .Finalize();

  int id = sim.Run();

  QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 35.0);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 50.0);

}

// A random number pulled from a uniform integer distribution can be
// implemented as the request size
TEST_F(SinkTest, RandomUniformSize) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_size_type>UniformReal</random_size_type> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Resources", NULL);
  EXPECT_EQ(qr.rows.size(), 1);
  // Given the PRNG with default seed, the resource should have mass 9.41273
  EXPECT_NEAR(qr.GetVal<double>("Quantity"), 9.41273, 0.0001);
}

// A random number pulled from a normal int distribution with default mean and
// stddev can be implemented as the request size
TEST_F(SinkTest, RandomNormalSize) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_size_type>NormalReal</random_size_type> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Resources", NULL);
  EXPECT_EQ(qr.rows.size(), 1);
  // Given the PRNG with default seed, the resource should have mass 9.60929
  EXPECT_NEAR(qr.GetVal<double>("Quantity"), 9.60929, 0.0001);
}

// A random number pulled from a normal int distribution with user-defined mean
// and stddev can be implemented as the request size
TEST_F(SinkTest, RandomNormalSizeWithMeanSttdev) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_size_type>NormalReal</random_size_type> "
    "   <random_size_mean>0.5</random_size_mean> "
    "   <random_size_stddev>0.2</random_size_stddev> ";

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Resources", NULL);
  EXPECT_EQ(qr.rows.size(), 1);
  // Given the PRNG with default seed, the resource should have mass 1.52979
  EXPECT_NEAR(qr.GetVal<double>("Quantity"), 1.52979, 0.0001);
}

// A random number pulled from a uniform integer distribution can be
// implemented as the buying frequency
TEST_F(SinkTest, RandomUniformFreq) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_frequency_type>UniformInt</random_frequency_type> "
    "   <random_frequency_min>2</random_frequency_min> "
    "   <random_frequency_max>4</random_frequency_max> ";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  // only one transaction has occurred
  EXPECT_EQ(qr.rows.size(), 1);
  // Get the time from the first transaction in the database (0th entry)
  int trans_time = qr.GetVal<int>("Time", 0);
  // Given the PRNG with default seed , this time should be time step 2
  EXPECT_EQ(trans_time, 2);
}

// A random number pulled from a normal int distribution with default mean and
// stddev can be implemented as the buying frequency
TEST_F(SinkTest, RandomNormalFreq) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_frequency_type>NormalInt</random_frequency_type> ";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  // only one transaction has occurred
  EXPECT_EQ(qr.rows.size(), 1);
  // Get the time from the first transaction in the database (0th entry)
  int trans_time = qr.GetVal<int>("Time", 0);
  // Given the PRNG with default seed , this time should be time step 2
  EXPECT_EQ(trans_time, 2);
}

// A random number pulled from a normal int distribution with user-defined mean
// and stddev can be implemented as the buying frequency
TEST_F(SinkTest, RandomNormalFreqWithMeanSttdev) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_frequency_type>NormalInt</random_frequency_type> "
    "   <random_frequency_mean>2</random_frequency_mean> "
    "   <random_frequency_stddev>0.2</random_frequency_stddev> ";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  // only one transaction has occurred
  EXPECT_EQ(qr.rows.size(), 1);
  // Get the time from the first transaction in the database (0th entry)
  int trans_time = qr.GetVal<int>("Time", 0);
  // Given the PRNG with default seed, this time should be time step 2
  EXPECT_EQ(trans_time, 2);
}

// Check that multiple buying cycles set by random number execute as expected
TEST_F(SinkTest, RandomNormalFreqMultipleCycles) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_frequency_type>NormalInt</random_frequency_type> "
    "   <random_frequency_mean>4</random_frequency_mean> "
    "   <random_frequency_stddev>1</random_frequency_stddev> ";

  int simdur = 12;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(10).Finalize();
  int id = sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  // three transaction should have occurred
  EXPECT_EQ(3, qr.rows.size());
  // check multiple cycles execute at the expected time
  // Get the time from the first, second, and third transactions in the
  // database (0th, 1st, and 2nd entry)
  // Given the PRNG with default seed, buy times on time step 5, 7, and 10
  int first_trans_time = qr.GetVal<int>("Time", 0);
  EXPECT_EQ(5, first_trans_time);
  int second_trans_time = qr.GetVal<int>("Time", 1);
  EXPECT_EQ(7, second_trans_time);
  int third_trans_time = qr.GetVal<int>("Time", 2);
  EXPECT_EQ(11, third_trans_time);  
}

// Check that randomness can be implemented in both size of request and
// request frequency at the same time
TEST_F(SinkTest, RandomNormalSizeUniformFreq) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "   <in_commods>"
    "     <val>commods_1</val>"
    "   </in_commods>"
    "   <capacity>10</capacity>"
    "   <random_size_type>NormalReal</random_size_type>"
    "   <random_size_mean>0.8</random_size_mean>"
    "   <random_size_stddev>0.2</random_size_stddev>"
    "   <random_frequency_type>UniformInt</random_frequency_type> "
    "   <random_frequency_min>2</random_frequency_min> "
    "   <random_frequency_max>4</random_frequency_max> ";

  int simdur = 6;
  cyclus::MockSim sim(cyclus::AgentSpec
          (":cycamore:Sink"), config, simdur);
  sim.AddSource("commods_1").capacity(20).Finalize();
  int id = sim.Run();

  QueryResult tqr = sim.db().Query("Transactions", NULL);
  // two transactions should have occurred
  EXPECT_EQ(2, tqr.rows.size());
  // check multiple cycles execute at the expected time
  int trans_time = tqr.GetVal<int>("Time", 0);
  EXPECT_EQ(3, trans_time);
  int res_id = tqr.GetVal<int>("ResourceId", 0);
  QueryResult rqr = sim.db().Query("Resources", NULL);
  double quantity = rqr.GetVal<double>("Quantity", 0);
  EXPECT_NEAR(6.54143, quantity, 0.00001);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SinkConstructor(cyclus::Context* ctx) {
  return new cycamore::Sink(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_SUITE_P(SinkFac, FacilityTests, Values(&SinkConstructor));
INSTANTIATE_TEST_SUITE_P(SinkFac, AgentTests, Values(&SinkConstructor));
