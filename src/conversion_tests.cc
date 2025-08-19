#include "conversion_tests.h"

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"
#include "pyne.h"

using pyne::nucname::id;

namespace cycamore {

void ConversionTest::SetUp() {
  conv_facility = new cycamore::Conversion(tc.get());
  trader = tc.trader();
  InitParameters();
  SetUpConversion();
  conv_facility->Build(NULL);
}

void ConversionTest::TearDown() {
  delete conv_facility;
}

void ConversionTest::InitParameters() {
  incommod1 = "incommod1";
  outcommod_name = "outcommod";
  throughput_val = DEFAULT_THROUGHPUT;
  input_capacity_val = DEFAULT_INPUT_CAPACITY;

  cyclus::CompMap v;
  v[id("u235")] = 1;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc.get()->AddRecipe("test_recipe", recipe);
}

void ConversionTest::SetUpConversion() {
  std::vector<std::string> incommods_vec;
  incommods_vec.push_back(incommod1);
  
  incommods(conv_facility, incommods_vec);
  outcommod(conv_facility, outcommod_name);
  throughput(conv_facility, throughput_val);
  input_capacity(conv_facility, input_capacity_val);
  
  // Set the actual buffer capacity
  set_input_capacity(conv_facility, input_capacity_val);
}

boost::shared_ptr<cyclus::ExchangeContext<cyclus::Material> > 
ConversionTest::GetContext(int nreqs, std::string commodity) {
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ExchangeContext;

  boost::shared_ptr<ExchangeContext<Material> > ec(
      new ExchangeContext<Material>());

  for (int i = 0; i < nreqs; i++) {
    Material::Ptr mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
    Request<Material>* req = Request<Material>::Create(mat, trader, commodity);
    ec->AddRequest(req);
  }

  return ec;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Clone) {
  cycamore::Conversion* cloned_fac = dynamic_cast<cycamore::Conversion*>
                                         (conv_facility->Clone());

  EXPECT_EQ(incommods(conv_facility), incommods(cloned_fac));
  EXPECT_EQ(outcommod(conv_facility), outcommod(cloned_fac));
  EXPECT_EQ(throughput(conv_facility), throughput(cloned_fac));
  EXPECT_EQ(input_capacity(conv_facility), input_capacity(cloned_fac));

  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Print) {
  EXPECT_NO_THROW(std::string s = conv_facility->str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, InitialState) {
  conv_facility->EnterNotify();
  
  EXPECT_EQ(throughput_val, throughput(conv_facility));
  EXPECT_EQ(input_capacity_val, input_capacity(conv_facility));
  EXPECT_EQ(outcommod_name, outcommod(conv_facility));
  
  std::vector<std::string> expected_incommods;
  expected_incommods.push_back(incommod1);
  EXPECT_EQ(expected_incommods, incommods(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, AvailableFeedstockCapacity) {
  conv_facility->EnterNotify();
  
  // Initially, capacity should be full
  EXPECT_DOUBLE_EQ(input_capacity_val, conv_facility->AvailableFeedstockCapacity());
  
  // Add some material to input buffer
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  input_push(conv_facility, mat);
  
  // Capacity should be reduced
  EXPECT_DOUBLE_EQ(input_capacity_val - TEST_QUANTITY, conv_facility->AvailableFeedstockCapacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlRequests) {
  using cyclus::RequestPortfolio;
  using cyclus::Material;
  using cyclus::CapacityConstraint;

  conv_facility->EnterNotify();

  std::set<RequestPortfolio<Material>::Ptr> ports =
      conv_facility->GetMatlRequests();

  ASSERT_EQ(1, ports.size());

  RequestPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(conv_facility, port->requester());

  // Check the portfolio for the request for incommod1 (should be the only one)
  EXPECT_EQ(1, port->requests().size());

  const std::set<CapacityConstraint<Material> >& constrs = port->constraints();
  ASSERT_EQ(1, constrs.size());
  EXPECT_EQ(CapacityConstraint<Material>(input_capacity_val), *constrs.begin());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlRequestsWhenFull) {
  using cyclus::RequestPortfolio;
  using cyclus::Material;

  conv_facility->EnterNotify();

  // Fill the input buffer
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(input_capacity_val);
  input_push(conv_facility, mat);

  std::set<RequestPortfolio<Material>::Ptr> ports =
      conv_facility->GetMatlRequests();

  // Should have no requests when buffer is full
  EXPECT_EQ(0, ports.size());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlBids) {
  using cyclus::BidPortfolio;
  using cyclus::Material;
  using cyclus::CapacityConstraint;
  using cyclus::CommodMap;

  conv_facility->EnterNotify();

  // Add some material to output buffer
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  output_push(conv_facility, mat);

  // Create commodity requests
  CommodMap<Material>::type commod_requests;
  cyclus::Material::Ptr req_mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  cyclus::Request<Material>* req = cyclus::Request<Material>::Create(
      req_mat, trader, outcommod_name);
  commod_requests[outcommod_name].push_back(req);

  std::set<BidPortfolio<Material>::Ptr> ports =
      conv_facility->GetMatlBids(commod_requests);

  // Should have one portfolio
  ASSERT_EQ(1, ports.size());

  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(conv_facility, port->bidder());

  // Should have one bid
  EXPECT_EQ(1, port->bids().size());

  // Check capacity constraint
  const std::set<CapacityConstraint<Material> >& constrs = port->constraints();
  ASSERT_EQ(1, constrs.size());
  EXPECT_EQ(CapacityConstraint<Material>(TEST_QUANTITY), *constrs.begin());

  delete req;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlBidsWhenEmpty) {
  using cyclus::BidPortfolio;
  using cyclus::Material;
  using cyclus::CommodMap;

  conv_facility->EnterNotify();

  // Create commodity requests
  CommodMap<Material>::type commod_requests;
  cyclus::Material::Ptr req_mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  cyclus::Request<Material>* req = cyclus::Request<Material>::Create(
      req_mat, trader, outcommod_name);
  commod_requests[outcommod_name].push_back(req);

  std::set<BidPortfolio<Material>::Ptr> ports =
      conv_facility->GetMatlBids(commod_requests);

  // Should have no bids when output buffer is empty
  EXPECT_EQ(0, ports.size());

  delete req;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, AcceptMatlTrades) {
  using cyclus::Material;
  using cyclus::Trade;
  using cyclus::Request;
  using cyclus::Bid;

  conv_facility->EnterNotify();

  // Create a trade
  Material::Ptr mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  Request<Material>* req = Request<Material>::Create(mat, trader, incommod1);
  Bid<Material>* bid = Bid<Material>::Create(req, mat, trader);
  Trade<Material> trade(req, bid, TEST_QUANTITY);

  std::vector<std::pair<Trade<Material>, Material::Ptr> > responses;
  responses.push_back(std::make_pair(trade, mat));

  // Accept the trade
  conv_facility->AcceptMatlTrades(responses);

  // Check that material was added to input buffer
  EXPECT_DOUBLE_EQ(TEST_QUANTITY, input_quantity(conv_facility));

  delete req;
  delete bid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlTrades) {
  using cyclus::Material;
  using cyclus::Trade;
  using cyclus::Request;
  using cyclus::Bid;

  conv_facility->EnterNotify();

  // Add material to output buffer
  Material::Ptr mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  output_push(conv_facility, mat);

  // Create a trade
  Material::Ptr req_mat = cyclus::NewBlankMaterial(TEST_QUANTITY);
  Request<Material>* req = Request<Material>::Create(req_mat, trader, outcommod_name);
  Bid<Material>* bid = Bid<Material>::Create(req, req_mat, trader);
  Trade<Material> trade(req, bid, TEST_QUANTITY);

  std::vector<Trade<Material> > trades;
  trades.push_back(trade);

  std::vector<std::pair<Trade<Material>, Material::Ptr> > responses;

  // Get the trade
  conv_facility->GetMatlTrades(trades, responses);

  // Check response
  ASSERT_EQ(1, responses.size());
  EXPECT_DOUBLE_EQ(TEST_QUANTITY, responses[0].second->quantity());

  // Check that material was removed from output buffer
  EXPECT_DOUBLE_EQ(0.0, output_quantity(conv_facility));

  delete req;
  delete bid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Convert) {
  conv_facility->EnterNotify();

  // Add material to input buffer (more than throughput)
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(DEFAULT_THROUGHPUT * 2);
  input_push(conv_facility, mat);

  // Convert material
  conv_facility->Convert();

  // Check that material was moved from input to output
  EXPECT_DOUBLE_EQ(DEFAULT_THROUGHPUT, input_quantity(conv_facility));  
  EXPECT_DOUBLE_EQ(DEFAULT_THROUGHPUT, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, ConvertWithLessThanThroughput) {
  conv_facility->EnterNotify();

  // Add material to input buffer (less than throughput)
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(DEFAULT_THROUGHPUT / 2);
  input_push(conv_facility, mat);

  // Convert material
  conv_facility->Convert();

  // Check that all material was moved from input to output
  EXPECT_DOUBLE_EQ(0.0, input_quantity(conv_facility)); 
  EXPECT_DOUBLE_EQ(DEFAULT_THROUGHPUT / 2, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, ConvertWithEmptyInput) {
  conv_facility->EnterNotify();

  // Convert with empty input buffer
  conv_facility->Convert();

  // Check that nothing happened
  EXPECT_DOUBLE_EQ(0.0, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(0.0, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Tick) {
  conv_facility->EnterNotify();

  // Add material to input buffer (more than throughput)
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(DEFAULT_THROUGHPUT * 2);
  input_push(conv_facility, mat);

  // Tick should trigger conversion
  conv_facility->Tick();

  // Check that material was converted
  EXPECT_DOUBLE_EQ(DEFAULT_THROUGHPUT, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(DEFAULT_THROUGHPUT, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, PositionInitialize) {
  std::string config = DEFAULT_CONFIG;
  int simdur = SIMULATION_DURATION;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Conversion"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), DEFAULT_POSITION);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), DEFAULT_POSITION);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, PositionWithCoordinates) {
  std::string config = DEFAULT_CONFIG + 
    "<latitude>" + std::to_string(TEST_POSITION) + "</latitude>"
    "<longitude>" + std::to_string(TEST_POSITION) + "</longitude>";
  int simdur = SIMULATION_DURATION;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Conversion"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), TEST_POSITION);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), TEST_POSITION);
}

}  // namespace cycamore 