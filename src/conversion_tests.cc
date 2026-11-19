#include "conversion_tests.h"

#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"
#include "pyne.h"
#include "xml_parser.h"

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
  throughput_val = kDefaultThroughput;
  input_capacity_val = kDefaultInputCapacity;

  cyclus::CompMap v;
  v[id("u235")] = 1;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc.get()->AddRecipe("test_recipe", recipe);
}

void ConversionTest::SetUpConversion() {
  std::vector<std::string> incommods_vec;
  incommods_vec.push_back(kInputCommodity1);
  
  incommods(conv_facility, incommods_vec);
  outcommod(conv_facility, kOutputCommodity);
  throughput(conv_facility, throughput_val);
  input_capacity(conv_facility, input_capacity_val);
  unit_cost(conv_facility, kTestUnitCost);
  unit_cost_mods(conv_facility, {});
  
  // Set the actual buffer capacity
  set_input_capacity(conv_facility, input_capacity_val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Clone) {
  unit_cost_mods(conv_facility, {-2.0});
  cycamore::Conversion* cloned_fac = dynamic_cast<cycamore::Conversion*>
                                         (conv_facility->Clone());

  EXPECT_EQ(incommods(conv_facility), incommods(cloned_fac));
  EXPECT_EQ(outcommod(conv_facility), outcommod(cloned_fac));
  EXPECT_EQ(throughput(conv_facility), throughput(cloned_fac));
  EXPECT_EQ(input_capacity(conv_facility), input_capacity(cloned_fac));
  EXPECT_EQ(unit_cost(conv_facility), unit_cost(cloned_fac));
  EXPECT_EQ(unit_cost_mods(conv_facility), unit_cost_mods(cloned_fac));

  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Print) {
  EXPECT_NO_THROW(std::string s = conv_facility->str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, InitialState) {
  const std::vector<std::string> expected_incommods = {kInputCommodity1, "alternative"};
  const std::vector<double> expected_modifiers(expected_incommods.size(),
                                               cyclus::kDefaultUnitCostMod);
  incommods(conv_facility, expected_incommods);
  // Build already initialized modifiers for the fixture's original commodity.
  unit_cost_mods(conv_facility, {});
  conv_facility->EnterNotify();

  EXPECT_EQ(throughput_val, throughput(conv_facility));
  EXPECT_EQ(input_capacity_val, input_capacity(conv_facility));
  EXPECT_EQ(kOutputCommodity, outcommod(conv_facility));
  EXPECT_EQ(expected_incommods, incommods(conv_facility));
  EXPECT_EQ(expected_modifiers, unit_cost_mods(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, AvailableFeedstockCapacity) {
  conv_facility->EnterNotify();
  
  // Initially, capacity should be full
  EXPECT_DOUBLE_EQ(input_capacity_val, conv_facility->AvailableFeedstockCapacity());
  
  // Add some material to input buffer
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(kTestQuantity);
  input_push(conv_facility, mat);
  
  // Capacity should be reduced
  EXPECT_DOUBLE_EQ(input_capacity_val - kTestQuantity, conv_facility->AvailableFeedstockCapacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, GetMatlRequests) {
  using cyclus::RequestPortfolio;
  using cyclus::Material;
  using cyclus::CapacityConstraint;

  const std::vector<std::string> input_commodities = {kInputCommodity1, "alternative"};
  const std::vector<double> expected_modifiers = {7.0, -2.0};
  incommods(conv_facility, input_commodities);
  unit_cost_mods(conv_facility, expected_modifiers);
  conv_facility->EnterNotify();

  auto ports = conv_facility->GetMatlRequests();
  ASSERT_EQ(1, ports.size());
  RequestPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(conv_facility, port->requester());

  const auto& requests = port->requests();
  ASSERT_EQ(input_commodities.size(), requests.size());
  for (std::size_t i = 0; i < requests.size(); ++i) {
    EXPECT_EQ(input_commodities[i], requests[i]->commodity());
    EXPECT_DOUBLE_EQ(expected_modifiers[i], requests[i]->unit_cost_mod());
    // Requests fill shared feed storage, even when it exceeds processing capacity.
    EXPECT_DOUBLE_EQ(input_capacity_val, requests[i]->target()->quantity());
  }
  EXPECT_DOUBLE_EQ(input_capacity_val, port->qty());
  const auto& constraints = port->constraints();
  ASSERT_EQ(1, constraints.size());
  EXPECT_EQ(CapacityConstraint<Material>(input_capacity_val), *constraints.begin());
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
  const std::vector<double> batch_unit_values = {2.0, 6.0};
  const double batch_quantity = kDefaultThroughput;
  const double total_quantity = batch_quantity * batch_unit_values.size();
  double total_value = 0;
  for (double value : batch_unit_values) {
    auto mat = Material::CreateUntracked(batch_quantity, recipe);
    mat->unit_value(value);
    output_push(conv_facility, mat);
    total_value += batch_quantity * value;
  }
  const double blended_unit_value = total_value / total_quantity;
  const double expected_bid_cost = blended_unit_value + unit_cost(conv_facility);

  CommodMap<Material>::type commod_requests;
  auto req_mat = Material::CreateUntracked(total_quantity, recipe);
  auto req = cyclus::Request<Material>::Create(req_mat, trader, kOutputCommodity);
  commod_requests[kOutputCommodity].push_back(req);
  auto ports = conv_facility->GetMatlBids(commod_requests);

  ASSERT_EQ(1, ports.size());
  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(conv_facility, port->bidder());
  ASSERT_EQ(1, port->bids().size());
  auto bid = *port->bids().begin();
  EXPECT_DOUBLE_EQ(expected_bid_cost, bid->unit_cost());
  // Accumulated output can ship beyond one timestep's processing capacity.
  EXPECT_DOUBLE_EQ(total_quantity, bid->offer()->quantity());
  EXPECT_DOUBLE_EQ(blended_unit_value, output_peek(conv_facility)->unit_value());

  const auto& constraints = port->constraints();
  ASSERT_EQ(1, constraints.size());
  EXPECT_EQ(CapacityConstraint<Material>(total_quantity), *constraints.begin());
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
  cyclus::Material::Ptr req_mat = cyclus::NewBlankMaterial(kTestQuantity);
  cyclus::Request<Material>* req = cyclus::Request<Material>::Create(
      req_mat, trader, kOutputCommodity);
  commod_requests[kOutputCommodity].push_back(req);

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
  Material::Ptr mat = cyclus::NewBlankMaterial(kTestQuantity);
  Request<Material>* req = Request<Material>::Create(mat, trader, kInputCommodity1);
  Bid<Material>* bid = Bid<Material>::Create(req, mat, trader);
  Trade<Material> trade(req, bid, kTestQuantity);

  std::vector<std::pair<Trade<Material>, Material::Ptr> > responses;
  responses.push_back(std::make_pair(trade, mat));

  // Accept the trade
  conv_facility->AcceptMatlTrades(responses);

  // Check that material was added to input buffer
  EXPECT_DOUBLE_EQ(kTestQuantity, input_quantity(conv_facility));

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
  Material::Ptr mat = cyclus::NewBlankMaterial(kTestQuantity);
  output_push(conv_facility, mat);

  // Create a trade
  Material::Ptr req_mat = cyclus::NewBlankMaterial(kTestQuantity);
  Request<Material>* req = Request<Material>::Create(req_mat, trader, kOutputCommodity);
  Bid<Material>* bid = Bid<Material>::Create(req, req_mat, trader);
  Trade<Material> trade(req, bid, kTestQuantity);

  std::vector<Trade<Material> > trades;
  trades.push_back(trade);

  std::vector<std::pair<Trade<Material>, Material::Ptr> > responses;

  // Get the trade
  conv_facility->GetMatlTrades(trades, responses);

  // Check response
  ASSERT_EQ(1, responses.size());
  EXPECT_DOUBLE_EQ(kTestQuantity, responses[0].second->quantity());

  // Check that material was removed from output buffer
  EXPECT_DOUBLE_EQ(0.0, output_quantity(conv_facility));

  delete req;
  delete bid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Convert) {
  conv_facility->EnterNotify();
  const double feed_quantity = 2 * kDefaultThroughput;
  const double feed_unit_value = 12.0;
  const double u235_mass_fraction = 0.00711;
  cyclus::CompMap uranium;
  uranium[id("u235")] = u235_mass_fraction;
  uranium[id("u238")] = 1.0 - u235_mass_fraction;
  auto comp = cyclus::Composition::CreateFromMass(uranium);
  auto feed = cyclus::Material::CreateUntracked(feed_quantity, comp);
  feed->unit_value(feed_unit_value);
  input_push(conv_facility, feed);

  conv_facility->Convert();

  EXPECT_DOUBLE_EQ(feed_quantity - kDefaultThroughput, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(kDefaultThroughput, output_quantity(conv_facility));
  EXPECT_EQ(comp->mass(), output_peek(conv_facility)->comp()->mass());
  EXPECT_DOUBLE_EQ(feed_unit_value, output_peek(conv_facility)->unit_value());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, ConvertWithLessThanThroughput) {
  conv_facility->EnterNotify();

  // Add material to input buffer (less than throughput)
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(kDefaultThroughput / 2);
  input_push(conv_facility, mat);

  // Convert material
  conv_facility->Convert();

  // Check that all material was moved from input to output
  EXPECT_DOUBLE_EQ(0.0, input_quantity(conv_facility)); 
  EXPECT_DOUBLE_EQ(kDefaultThroughput / 2, output_quantity(conv_facility));
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
TEST_F(ConversionTest, ZeroThroughput) {
  throughput(conv_facility, 0);
  input_push(conv_facility, cyclus::Material::CreateUntracked(kTestQuantity, recipe));
  EXPECT_NO_THROW(conv_facility->Tick());
  EXPECT_DOUBLE_EQ(kTestQuantity, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(0, output_quantity(conv_facility));
}

TEST_F(ConversionTest, InventorySnapshotRestore) {
  const double feed_quantity = 2 * kDefaultThroughput;
  const double feed_unit_value = 12.0;
  const double expected_inventory = kDefaultThroughput;
  auto feed = cyclus::Material::CreateUntracked(feed_quantity, recipe);
  feed->unit_value(feed_unit_value);
  input_push(conv_facility, feed);
  conv_facility->Convert();
  cyclus::Inventories inv = conv_facility->SnapshotInv();
  ASSERT_EQ(1, inv["input"].size());
  ASSERT_EQ(1, inv["output"].size());
  // Snapshot must also leave the running facility's inventory intact.
  EXPECT_DOUBLE_EQ(expected_inventory, output_quantity(conv_facility));
  Conversion restored(tc.get());
  restored.InitInv(inv);
  EXPECT_DOUBLE_EQ(expected_inventory, input_quantity(&restored));
  EXPECT_DOUBLE_EQ(expected_inventory, output_quantity(&restored));
  EXPECT_EQ(recipe->mass(), output_peek(&restored)->comp()->mass());
  EXPECT_DOUBLE_EQ(feed_unit_value, output_peek(&restored)->unit_value());
}

TEST_F(ConversionTest, Tick) {
  conv_facility->EnterNotify();

  // Add material to input buffer (more than throughput)
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(kDefaultThroughput * 2);
  input_push(conv_facility, mat);

  // Tick should trigger conversion
  conv_facility->Tick();

  // Check that material was converted
  EXPECT_DOUBLE_EQ(kDefaultThroughput, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(kDefaultThroughput, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, Tock) {
  conv_facility->EnterNotify();

  // Add material to input buffer
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(kDefaultThroughput);
  input_push(conv_facility, mat);

  // Tock should not trigger conversion
  conv_facility->Tock();

  // Check that no conversion happened
  EXPECT_DOUBLE_EQ(kDefaultThroughput, input_quantity(conv_facility));
  EXPECT_DOUBLE_EQ(0.0, output_quantity(conv_facility));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, PositionInitialize) {
  std::string config = kDefaultConfig;
  int simdur = kSimulationDuration;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Conversion"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), kDefaultPosition);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), kDefaultPosition);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ConversionTest, PositionWithCoordinates) {
  std::string config = kDefaultConfig +
    "<latitude>" + std::to_string(kTestPosition) + "</latitude>"
    "<longitude>" + std::to_string(kTestPosition) + "</longitude>";
  int simdur = kSimulationDuration;
  cyclus::MockSim sim(cyclus::AgentSpec (":cycamore:Conversion"), config, simdur);
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), kTestPosition);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), kTestPosition);
}


// Validate the generated schema rather than relying on UI range annotations.
static void ValidateConversionConfig(Conversion* facility,
                                     const std::string& config) {
  std::stringstream schema;
  schema << "<element "
         << "xmlns=\"http://relaxng.org/ns/structure/1.0\" "
         << "xmlns:a=\"http://relaxng.org/ns/annotation/1.0\" "
         << "datatypeLibrary=\"http://www.w3.org/2001/XMLSchema-datatypes\" "
         << "name=\"config\">"
         << facility->schema()
         << "</element>";
  std::stringstream xml;
  xml << "<config>" << config << "</config>";
  cyclus::XMLParser parser;
  parser.Init(xml);
  parser.Validate(schema);
}

TEST_F(ConversionTest, NumericInputValidation) {
  EXPECT_NO_THROW(ValidateConversionConfig(conv_facility, kDefaultConfig));
  EXPECT_NO_THROW(ValidateConversionConfig(conv_facility,
      kDefaultConfig +
      "<unit_cost>0</unit_cost>"
      "<unit_cost_mods>"
        "<val>-2</val>"
      "</unit_cost_mods>"));
  const std::string throughput_close_tag = "</throughput>";
  std::string zero_config = kDefaultConfig;
  std::size_t zero_begin = zero_config.find("<throughput>");
  std::size_t zero_end = zero_config.find(throughput_close_tag) + throughput_close_tag.size();
  zero_config.replace(zero_begin, zero_end - zero_begin, "<throughput>0</throughput>");
  EXPECT_NO_THROW(ValidateConversionConfig(conv_facility, zero_config));
  const std::vector<std::string> nonfinite_values = {"NaN", "INF", "-INF"};
  std::vector<std::string> invalid_nonnegative_values = nonfinite_values;
  invalid_nonnegative_values.push_back("-1");
  invalid_nonnegative_values.push_back("1e300");  // Exceeds CY_LARGE_DOUBLE.
  for (const std::string& value : invalid_nonnegative_values) {
    SCOPED_TRACE(value);
    EXPECT_THROW(ValidateConversionConfig(conv_facility,
        kDefaultConfig + "<unit_cost>" + value + "</unit_cost>"),
        cyclus::ValidationError);
    std::string config = kDefaultConfig;
    std::size_t begin = config.find("<throughput>");
    std::size_t end = config.find(throughput_close_tag) + throughput_close_tag.size();
    config.replace(begin, end - begin, "<throughput>" + value + "</throughput>");
    EXPECT_THROW(ValidateConversionConfig(conv_facility, config),
                 cyclus::ValidationError);
  }
  for (const std::string& value : nonfinite_values) {
    EXPECT_THROW(ValidateConversionConfig(conv_facility,
        kDefaultConfig +
        "<unit_cost_mods>"
          "<val>" + value + "</val>"
        "</unit_cost_mods>"), cyclus::ValidationError);
  }
}

TEST_F(ConversionTest, RequestModifierSizeMismatch) {
  unit_cost_mods(conv_facility, {1.0, 2.0});
  EXPECT_THROW(conv_facility->EnterNotify(), cyclus::ValueError);
}

TEST_F(ConversionTest, CostFlowXML) {
  const double conversion_unit_cost = kTestUnitCost;
  const double input_cost_modifier = -2.0;
  std::stringstream config;
  config << kDefaultConfig
         << "<unit_cost>" << conversion_unit_cost << "</unit_cost>"
         << "<unit_cost_mods>"
           << "<val>" << input_cost_modifier << "</val>"
         << "</unit_cost_mods>";
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Conversion"), config.str(),
                      kSimulationDuration);
  sim.AddSource(kInputCommodity1).capacity(kDefaultThroughput).Finalize();
  sim.AddSink(kOutputCommodity).Finalize();
  int agent_id = sim.Run();
  std::vector<cyclus::Cond> conds;
  conds.push_back(cyclus::Cond("ReceiverId", "==", agent_id));
  auto incoming = sim.db().Query("Transactions", &conds);
  ASSERT_GT(incoming.rows.size(), 0);
  EXPECT_DOUBLE_EQ(input_cost_modifier, incoming.GetVal<double>("UnitCostMod", 0));
  const double expected_output_cost =
      incoming.GetVal<double>("UnitCost", 0) + conversion_unit_cost;
  conds.clear();
  conds.push_back(cyclus::Cond("SenderId", "==", agent_id));
  auto outgoing = sim.db().Query("Transactions", &conds);
  // Newly received feed waits until next Tick.
  const int expected_shipments = kSimulationDuration - 1;
  ASSERT_EQ(expected_shipments, outgoing.rows.size());
  for (std::size_t i = 0; i < outgoing.rows.size(); ++i) {
    EXPECT_DOUBLE_EQ(expected_output_cost, outgoing.GetVal<double>("UnitCost", i));
    EXPECT_DOUBLE_EQ(outgoing.GetVal<double>("UnitCost", i),
                     outgoing.GetVal<double>("ArcCost", i));
  }
}

TEST_F(ConversionTest, RequestModifiersSelectCommodity) {
  const std::string higher_cost_commodity = "higher_cost";
  const std::string lower_cost_commodity = "lower_cost";
  const double higher_cost_modifier = 7.0;
  const double lower_cost_modifier = -2.0;
  const double feed_capacity = kTestQuantity;
  std::stringstream config;
  config << "<incommods>"
           << "<val>" << higher_cost_commodity << "</val>"
           << "<val>" << lower_cost_commodity << "</val>"
         << "</incommods>"
         << "<outcommod>" << kOutputCommodity << "</outcommod>"
         << "<input_capacity>" << feed_capacity << "</input_capacity>"
         << "<unit_cost_mods>"
           << "<val>" << higher_cost_modifier << "</val>"
           << "<val>" << lower_cost_modifier << "</val>"
         << "</unit_cost_mods>";
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Conversion"), config.str(), 1);
  sim.AddSource(higher_cost_commodity).capacity(feed_capacity).Finalize();
  sim.AddSource(lower_cost_commodity).capacity(feed_capacity).Finalize();
  int agent_id = sim.Run();
  std::vector<cyclus::Cond> conds;
  conds.push_back(cyclus::Cond("ReceiverId", "==", agent_id));
  auto incoming = sim.db().Query("Transactions", &conds);
  ASSERT_EQ(1, incoming.rows.size());
  EXPECT_EQ(lower_cost_commodity, incoming.GetVal<std::string>("Commodity", 0));
  EXPECT_DOUBLE_EQ(lower_cost_modifier, incoming.GetVal<double>("UnitCostMod", 0));
}

TEST_F(ConversionTest, DefaultCostXML) {
  const double expected_default_unit_cost = 1.0;
  const int simulation_duration = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Conversion"), kDefaultConfig,
                      simulation_duration);
  sim.AddSource(kInputCommodity1).capacity(kDefaultThroughput).Finalize();
  sim.AddSink(kOutputCommodity).Finalize();
  int agent_id = sim.Run();
  std::vector<cyclus::Cond> conds;
  conds.push_back(cyclus::Cond("SenderId", "==", agent_id));
  auto outgoing = sim.db().Query("Transactions", &conds);
  ASSERT_EQ(1, outgoing.rows.size());
  EXPECT_DOUBLE_EQ(cyclus::kDefaultUnitCost + expected_default_unit_cost,
                   outgoing.GetVal<double>("UnitCost", 0));
}

}  // namespace cycamore 