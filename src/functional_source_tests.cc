#include "functional_source_tests.h"

#include <algorithm>

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"

namespace cycamore {

void FunctionalSourceTest::SetUp() {
  src_facility = new cycamore::FunctionalSource(tc.get());
  trader = tc.trader();
  InitParameters();
  SetUpFunctionalSource();
  src_facility->Build(NULL);
}

void FunctionalSourceTest::TearDown() {
  delete src_facility;
}

void FunctionalSourceTest::InitParameters() {
  commod = "commod";
  recipe_name = "recipe";
  package_name = "testpackage";
  tu_name = "testtu";
  function = "linear";
  slope = 0;      // zero to recreate constant throughput
  intercept = 5;  // some magic number
  params = {slope, intercept};

  recipe = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  tc.get()->AddRecipe(recipe_name, recipe);
}

void FunctionalSourceTest::SetUpFunctionalSource() {
  outcommod(src_facility, commod);
  outrecipe(src_facility, recipe_name);
  throughput_function(src_facility, function);
  throughput_params(src_facility, params);
}

TEST_F(FunctionalSourceTest, Clone) {
  cyclus::Context* ctx = tc.get();
  cycamore::FunctionalSource* cloned_fac = dynamic_cast<cycamore::FunctionalSource*>
                                         (src_facility->Clone());

  EXPECT_EQ(outcommod(src_facility),  outcommod(cloned_fac));
  EXPECT_EQ(outrecipe(src_facility),  outrecipe(cloned_fac));
  EXPECT_EQ(throughput_function(src_facility), throughput_function(cloned_fac));
  EXPECT_EQ(throughput_params(src_facility), throughput_params(cloned_fac));

  delete cloned_fac;
}

TEST_F(FunctionalSourceTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

TEST_F(FunctionalSourceTest, InvalidThroughputFunctionError) {
  const std::string commodity = "commod";
  const std::string throughput_function = "invalid";
  const double slope = 1;
  const double intercept = 1;
  std::stringstream config;
  config << "<outcommod>" << commodity << "</outcommod>"
         << "<throughput_function>" << throughput_function << "</throughput_function>"
         << "<throughput_params>"
         << "<val>" << slope << "</val>"
         << "<val>" << intercept << "</val>"
         << "</throughput_params>";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config.str(), simdur);

  EXPECT_THROW(sim.Run(), cyclus::ValueError);
}

TEST_F(FunctionalSourceTest, ThroughputParamsError) {
  const std::string commodity = "commod";
  const std::string linear_function = "linear";
  const std::string exponential_function = "exponential";
  const double single_param = 1;
  std::stringstream linear_config;
  linear_config << "<outcommod>" << commodity << "</outcommod>"
                << "<throughput_function>" << linear_function << "</throughput_function>"
                << "<throughput_params>"
                << "<val>" << single_param << "</val>"
                << "</throughput_params>";
  std::stringstream exponential_config;
  exponential_config << "<outcommod>" << commodity << "</outcommod>"
                     << "<throughput_function>" << exponential_function << "</throughput_function>"
                     << "<throughput_params>"
                     << "<val>" << single_param << "</val>"
                     << "</throughput_params>";
  int simdur = 3;

  cyclus::MockSim linear_sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), linear_config.str(), simdur);
  EXPECT_THROW(linear_sim.Run(), cyclus::ValueError);

  cyclus::MockSim exponential_sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), exponential_config.str(), simdur);
  EXPECT_THROW(exponential_sim.Run(), cyclus::ValueError);
}

TEST_F(FunctionalSourceTest, LinearFunctionCalculatesExpectedThroughput) {
  const std::string commodity = "commod";
  const std::string throughput_function = "linear";
  const double slope = 2;
  const double intercept = 2;
  std::stringstream config;
  config << "<outcommod>" << commodity << "</outcommod>"
         << "<throughput_function>" << throughput_function << "</throughput_function>"
         << "<throughput_params>"
         << "<val>" << slope << "</val>"
         << "<val>" << intercept << "</val>"
         << "</throughput_params>";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config.str(), simdur);
  sim.AddSink(commodity).Finalize();

  int id = sim.Run();

  cyclus::QueryResult qr =
    sim.db().Query("TimeSeriessupply" + commodity, NULL);
  ASSERT_EQ(simdur, qr.rows.size());
  for (int i = 0; i < simdur; ++i) {
    double expected_throughput = slope * i + intercept;
    EXPECT_DOUBLE_EQ(expected_throughput, qr.GetVal<double>("Value", i));
  }
}

TEST_F(FunctionalSourceTest, ExponentialFunctionCalculatesExpectedThroughput) {
  const std::string commodity = "commod";
  const std::string throughput_function = "exponential";
  const double constant = 2;
  const double exponent = 2;
  const double intercept = 2;
  std::stringstream config;
  config << "<outcommod>" << commodity << "</outcommod>"
         << "<throughput_function>" << throughput_function << "</throughput_function>"
         << "<throughput_params>"
         << "<val>" << constant << "</val>"
         << "<val>" << exponent << "</val>"
         << "<val>" << intercept << "</val>"
         << "</throughput_params>";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config.str(), simdur);
  sim.AddSink(commodity).Finalize();

  int id = sim.Run();

  cyclus::QueryResult qr =
    sim.db().Query("TimeSeriessupply" + commodity, NULL);
  ASSERT_EQ(simdur, qr.rows.size());
  for (int i = 0; i < simdur; ++i) {
    double expected_throughput = constant * std::exp(exponent * i) + intercept;
    EXPECT_DOUBLE_EQ(expected_throughput, qr.GetVal<double>("Value", i));
  }
}

TEST_F(FunctionalSourceTest, ZeroSlopeLinearMatchesSource) {
  const std::string commodity = "commod";
  const std::string throughput_function = "linear";
  const double slope = 0;
  const double intercept = 2;
  std::stringstream func_config;
  func_config << "<outcommod>" << commodity << "</outcommod>"
              << "<throughput_function>" << throughput_function << "</throughput_function>"
              << "<throughput_params>"
              << "<val>" << slope << "</val>"
              << "<val>" << intercept << "</val>"
              << "</throughput_params>";
  std::stringstream src_config;
  src_config << "<outcommod>" << commodity << "</outcommod>"
             << "<throughput>" << intercept << "</throughput>";

  int simdur = 3;

  cyclus::MockSim func_sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), func_config.str(), simdur);
  cyclus::MockSim src_sim(cyclus::AgentSpec (":cycamore:Source"), src_config.str(), simdur);

  func_sim.Run();
  src_sim.Run();

  cyclus::QueryResult func_qr =
      func_sim.db().Query("TimeSeriessupplycommod", NULL);

  cyclus::QueryResult src_qr =
      src_sim.db().Query("TimeSeriessupplycommod", NULL);

  ASSERT_EQ(src_qr.rows.size(), func_qr.rows.size());

  for (int i = 0; i < simdur; ++i) {
    EXPECT_DOUBLE_EQ(
        src_qr.GetVal<double>("Value", i),
        func_qr.GetVal<double>("Value", i));
  }
}

TEST_F(FunctionalSourceTest, DefaultProductionCreatesInventoryAtBuild) {
  cycamore::FunctionalSource functional_source(tc.get());
  inventory_size(&functional_source, 12);
  throughput_function(&functional_source, "linear");
  throughput_params(&functional_source, params);

  EXPECT_FALSE(incremental_production(&functional_source));
  functional_source.Build(NULL);
  EXPECT_DOUBLE_EQ(12, inventory_quantity(&functional_source));
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(12, inventory_quantity(&functional_source));
}

TEST_F(FunctionalSourceTest, IncrementalProductionAccumulatesUpToInventorySize) {
  cycamore::FunctionalSource functional_source(tc.get());
  inventory_size(&functional_source, 12);
  throughput_function(&functional_source, "linear");
  throughput_params(&functional_source, params);
  incremental_production(&functional_source, true);

  functional_source.Build(NULL);
  EXPECT_DOUBLE_EQ(0, inventory_quantity(&functional_source));
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(5, inventory_quantity(&functional_source));
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(10, inventory_quantity(&functional_source));
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(12, inventory_quantity(&functional_source));
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(12, inventory_quantity(&functional_source));
}

TEST_F(FunctionalSourceTest, ClonePreservesIncrementalProduction) {
  incremental_production(src_facility, true);
  cycamore::FunctionalSource* clone = dynamic_cast<cycamore::FunctionalSource*>(src_facility->Clone());
  ASSERT_TRUE(clone != NULL);
  EXPECT_TRUE(incremental_production(clone));
  delete clone;
}

TEST_F(FunctionalSourceTest, IncrementalProductionLimitIncludesSuppliedMaterial) {
  cycamore::FunctionalSource functional_source(tc.get());
  inventory_size(&functional_source, 7);
  throughput_function(&functional_source, "linear");
  throughput_params(&functional_source, params);
  incremental_production(&functional_source, true);
  functional_source.Build(NULL);

  functional_source.Tick();
  remove_inventory(&functional_source, 5);
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(2, inventory_quantity(&functional_source));
  remove_inventory(&functional_source, 2);
  functional_source.Tick();
  EXPECT_DOUBLE_EQ(0, inventory_quantity(&functional_source));
}

TEST_F(FunctionalSourceTest, IncrementalProductionInput) {
  std::string config =
    "<outcommod>commod</outcommod>"
    "<inventory_size>12</inventory_size>"
    "<throughput_function>linear</throughput_function>"
    "<throughput_params><val>0</val><val>5</val></throughput_params>"
    "<incremental_production>true</incremental_production>";
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:FunctionalSource"), config, 3);
  sim.AddSink("commod").Finalize();

  EXPECT_NO_THROW(sim.Run());
  cyclus::QueryResult transactions = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(3, transactions.rows.size());
}

TEST_F(FunctionalSourceTest, AddBids) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::ExchangeContext;
  using cyclus::Material;

  int nreqs = 5;

  boost::shared_ptr< ExchangeContext<Material> >
      ec = GetContext(nreqs, commod);

  src_facility->EnterNotify();
  src_facility->Tick();

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
  EXPECT_EQ(*constrs.begin(), CapacityConstraint<Material>(intercept));
}

TEST_F(FunctionalSourceTest, Response) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::get_mat;

  std::vector< Trade<Material> > trades;
  std::vector<std::pair<Trade<Material>,
                        Material::Ptr> > responses;

  // Null response
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 0);

  double qty = intercept / 3;
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

TEST_F(FunctionalSourceTest, PositionInitialize) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
    "<throughput_function>linear</throughput_function>"
    "<throughput_params><val>0</val><val>5</val></throughput_params>";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);

}

TEST_F(FunctionalSourceTest, Longitude) {
  std::string config =
    "<outcommod>spent_fuel</outcommod>"
    "<latitude>-0.01</latitude>"
    "<longitude>0.01</longitude>"
    "<throughput_function>linear</throughput_function>"
    "<throughput_params><val>0</val><val>5</val></throughput_params>";
    
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), -0.01);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.01);

}

TEST_F(FunctionalSourceTest, Package) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "<outcommod>commod</outcommod>"
    "<outrecipe>recipe</outrecipe>"
    "<package>testpackage</package>"
    "<throughput_function>linear</throughput_function>"
    "<throughput_params><val>0</val><val>5</val></throughput_params>";

  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config, simdur);
  
  sim.context()->AddRecipe(recipe_name, recipe);
  sim.context()->AddPackage(package_name, 3, 4, "first");
  package = sim.context()->GetPackage(package_name);
  
  sim.AddSink("commod").Finalize();

  EXPECT_NO_THROW(sim.Run());

  QueryResult qr_tr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(qr_tr.rows.size(), 3);
  
  std::vector<Cond> conds;
  conds.push_back(Cond("PackageName", "==", package->name()));
  QueryResult qr_res = sim.db().Query("Resources", &conds);

  EXPECT_EQ(qr_res.rows.size(), 3); 
}

TEST_F(FunctionalSourceTest, TransportUnit) {
  using cyclus::QueryResult;
  using cyclus::Cond;

  std::string config =
    "<outcommod>commod</outcommod>"
    "<outrecipe>recipe</outrecipe>"
    "<package>testpackage</package>"
    "<transport_unit>testtu</transport_unit>"
    "<throughput_function>linear</throughput_function>"
    "<throughput_params><val>0</val><val>10</val></throughput_params>";

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:FunctionalSource"), config, simdur);
  
  sim.context()->AddRecipe(recipe_name, recipe);
  sim.context()->AddPackage(package_name, 3, 4, "equal");
  package = sim.context()->GetPackage(package_name);
  sim.context()->AddTransportUnit(tu_name, 2, 2);
  tu = sim.context()->GetTransportUnit(tu_name);
  
  sim.AddSink("commod").Finalize();

  EXPECT_NO_THROW(sim.Run());

  QueryResult qr_tr = sim.db().Query("Transactions", NULL);
  EXPECT_EQ(qr_tr.rows.size(), 4);
  
  std::vector<Cond> conds;
  conds.push_back(Cond("PackageName", "==", package->name()));
  QueryResult qr_res = sim.db().Query("Resources", &conds);

  EXPECT_EQ(qr_res.rows.size(), 4);

  QueryResult qr_allres = sim.db().Query("Resources", NULL);
}

boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
FunctionalSourceTest::GetContext(int nreqs, std::string commod) {
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

TEST_F(FunctionalSourceTest, TimeSeriesTests) {
  const std::string commodity = "commod";
  const double inventory_size = 8;
  const std::string throughput_function = "linear";
  const double slope = 0;
  const double intercept = 5;
  const double sink_throughput = 3;
  const double expected_throughput = 5;
  std::stringstream config;
  config << "<outcommod>" << commodity << "</outcommod>"
         << "<inventory_size>" << inventory_size << "</inventory_size>"
         << "<throughput_function>" << throughput_function << "</throughput_function>"
         << "<throughput_params>"
         << "<val>" << slope << "</val>"
         << "<val>" << intercept << "</val>"
         << "</throughput_params>";
  int simdur = 3;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:FunctionalSource"), config.str(),
                      simdur);
  // The source sells 3 kg in each of the first two steps. Its 5 kg throughput
  // controls the first two offers; the final offer is limited by inventory.
  sim.AddSink(commodity).capacity(sink_throughput).Finalize();
  int id = sim.Run();

  cyclus::QueryResult qr =
      sim.db().Query("TimeSeriessupply" + commodity, NULL);
  ASSERT_EQ(simdur, qr.rows.size());
  double inventory_remaining = inventory_size;
  for (int i = 0; i < simdur; ++i) {
    EXPECT_DOUBLE_EQ(std::min(expected_throughput, inventory_remaining),
                     qr.GetVal<double>("Value", i));
    inventory_remaining -= sink_throughput;
  }
}

} // namespace cycamore

cyclus::Agent* FunctionalSourceConstructor(cyclus::Context* ctx) {
  return new cycamore::FunctionalSource(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

INSTANTIATE_TEST_SUITE_P(FunctionalSourceFac, FacilityTests, Values(&FunctionalSourceConstructor));
INSTANTIATE_TEST_SUITE_P(FunctionalSourceFac, AgentTests, Values(&FunctionalSourceConstructor));
