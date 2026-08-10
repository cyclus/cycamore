#include "tariff_region_tests.h"

#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "bid.h"
#include "material.h"
#include "product.h"
#include "request.h"
#include "test_agents/test_facility.h"
#include "test_agents/test_inst.h"

namespace cycamore {
namespace {

class NamedRegion : public cyclus::Region {
 public:
  NamedRegion(cyclus::Context* ctx, const std::string& name)
      : cyclus::Region(ctx) {
    cyclus::Agent::prototype(name);
  }

  virtual cyclus::Agent* Clone() {
    return new NamedRegion(context(), prototype());
  }
};

}  // namespace

void TariffRegionTests::SetUp() {
  region_ = new TariffRegion(tc_.get());
}

void TariffRegionTests::TearDown() {
  delete region_;
}

void TariffRegionTests::SetAdjustment(const std::string& region,
                                      const std::string& commodity,
                                      double value,
                                      const std::string& type) {
  region_->adjustments_[region][commodity] = Adjustment(value, type);
}

Adjustment TariffRegionTests::FindAdjustment(
    cyclus::Region* supplier_region, const std::string& commodity) {
  return region_->FindAdjustmentForCommodity(supplier_region, commodity);
}

void TariffRegionTests::ValidateConfiguration() {
  region_->ValidateConfiguration();
}

TEST_F(TariffRegionTests, NoConfigurationMeansNoAdjustment) {
  NamedRegion supplier_region(tc_.get(), "Alpha");

  Adjustment adjustment = FindAdjustment(&supplier_region, "Fuel");

  EXPECT_DOUBLE_EQ(0.0, adjustment.first);
  EXPECT_EQ("unit_cost", adjustment.second);
  EXPECT_NO_THROW(ValidateConfiguration());
}

TEST_F(TariffRegionTests, WildcardPrecedence) {
  SetAdjustment("*", "*", 0.1);
  SetAdjustment("*", "Fuel", 0.2);
  SetAdjustment("Alpha", "*", 0.3);
  SetAdjustment("Alpha", "Fuel", 0.4);

  NamedRegion alpha(tc_.get(), "Alpha");
  NamedRegion beta(tc_.get(), "Beta");

  EXPECT_DOUBLE_EQ(0.4, FindAdjustment(&alpha, "Fuel").first);
  EXPECT_DOUBLE_EQ(0.3, FindAdjustment(&alpha, "Ore").first);
  EXPECT_DOUBLE_EQ(0.2, FindAdjustment(&beta, "Fuel").first);
  EXPECT_DOUBLE_EQ(0.1, FindAdjustment(&beta, "Ore").first);
}

TEST_F(TariffRegionTests, ExplicitZeroStopsWildcardFallback) {
  SetAdjustment("*", "Fuel", 0.5);
  SetAdjustment("Alpha", "*", 0.0);

  NamedRegion alpha(tc_.get(), "Alpha");
  Adjustment adjustment = FindAdjustment(&alpha, "Fuel");

  EXPECT_DOUBLE_EQ(0.0, adjustment.first);
  EXPECT_EQ("unit_cost", adjustment.second);
}

TEST_F(TariffRegionTests, ValidatesAdjustmentType) {
  SetAdjustment("Alpha", "Fuel", 0.1, "unit_cost");
  SetAdjustment("Beta", "Fuel", 0.2, "arc_cost");
  EXPECT_NO_THROW(ValidateConfiguration());

  SetAdjustment("Gamma", "Fuel", 0.3, "invalid");
  EXPECT_THROW(ValidateConfiguration(), cyclus::ValueError);
}

TEST_F(TariffRegionTests, CloneCopiesAdjustments) {
  SetAdjustment("Alpha", "Fuel", 0.1, "unit_cost");
  SetAdjustment("*", "*", 0.2, "arc_cost");

  TariffRegion* clone = dynamic_cast<TariffRegion*>(region_->Clone());

  ASSERT_NE(static_cast<TariffRegion*>(NULL), clone);
  EXPECT_EQ(region_->adjustments_, clone->adjustments_);
  delete clone;
}

TEST_F(TariffRegionTests, AdjustsMaterialUnitCost) {
  SetAdjustment("Alpha", "Fuel", 0.5, "unit_cost");

  NamedRegion supplier_region(tc_.get(), "Alpha");
  TestInst supplier_inst(tc_.get());
  TestFacility supplier(tc_.get());
  supplier_region.Build(NULL);
  supplier_inst.Build(&supplier_region);
  supplier.Build(&supplier_inst);

  cyclus::Request<cyclus::Material>* request =
      cyclus::Request<cyclus::Material>::Create(
          tc_.mat(), tc_.trader(), "Fuel");
  cyclus::Bid<cyclus::Material>* bid =
      cyclus::Bid<cyclus::Material>::Create(
          request, tc_.mat(), &supplier, false, 4.0);
  cyclus::RequestBidMap<cyclus::Material>::type costs;
  costs[request][bid] = 6.0;

  region_->AdjustMatlParams(costs);

  EXPECT_DOUBLE_EQ(6.0, bid->unit_cost());
  EXPECT_DOUBLE_EQ(8.0, costs[request][bid]);

  delete bid;
  delete request;
}

TEST_F(TariffRegionTests, AdjustsMaterialArcCost) {
  SetAdjustment("Alpha", "Fuel", 0.5, "arc_cost");

  NamedRegion supplier_region(tc_.get(), "Alpha");
  TestInst supplier_inst(tc_.get());
  TestFacility supplier(tc_.get());
  supplier_region.Build(NULL);
  supplier_inst.Build(&supplier_region);
  supplier.Build(&supplier_inst);

  cyclus::Request<cyclus::Material>* request =
      cyclus::Request<cyclus::Material>::Create(
          tc_.mat(), tc_.trader(), "Fuel");
  cyclus::Bid<cyclus::Material>* bid =
      cyclus::Bid<cyclus::Material>::Create(
          request, tc_.mat(), &supplier, false, 4.0);
  cyclus::RequestBidMap<cyclus::Material>::type costs;
  costs[request][bid] = 6.0;

  region_->AdjustMatlParams(costs);

  EXPECT_DOUBLE_EQ(4.0, bid->unit_cost());
  EXPECT_DOUBLE_EQ(9.0, costs[request][bid]);

  delete bid;
  delete request;
}

TEST_F(TariffRegionTests, AdjustsProducts) {
  SetAdjustment("Alpha", "Widgets", -0.25, "unit_cost");

  NamedRegion supplier_region(tc_.get(), "Alpha");
  TestInst supplier_inst(tc_.get());
  TestFacility supplier(tc_.get());
  supplier_region.Build(NULL);
  supplier_inst.Build(&supplier_region);
  supplier.Build(&supplier_inst);

  cyclus::Product::Ptr product =
      cyclus::Product::CreateUntracked(1.0, "widget");
  cyclus::Request<cyclus::Product>* request =
      cyclus::Request<cyclus::Product>::Create(
          product, tc_.trader(), "Widgets");
  cyclus::Bid<cyclus::Product>* bid =
      cyclus::Bid<cyclus::Product>::Create(
          request, product, &supplier, false, 8.0);
  cyclus::RequestBidMap<cyclus::Product>::type costs;
  costs[request][bid] = 10.0;

  region_->AdjustProductParams(costs);

  EXPECT_DOUBLE_EQ(6.0, bid->unit_cost());
  EXPECT_DOUBLE_EQ(8.0, costs[request][bid]);

  delete bid;
  delete request;
}

TEST_F(TariffRegionTests, DoesNotAdjustDomesticBid) {
  SetAdjustment("*", "*", 0.5, "unit_cost");

  TestInst supplier_inst(tc_.get());
  TestFacility supplier(tc_.get());
  region_->Build(NULL);
  supplier_inst.Build(region_);
  supplier.Build(&supplier_inst);

  cyclus::Request<cyclus::Material>* request =
      cyclus::Request<cyclus::Material>::Create(
          tc_.mat(), tc_.trader(), "Fuel");
  cyclus::Bid<cyclus::Material>* bid =
      cyclus::Bid<cyclus::Material>::Create(
          request, tc_.mat(), &supplier, false, 4.0);
  cyclus::RequestBidMap<cyclus::Material>::type costs;
  costs[request][bid] = 6.0;

  region_->AdjustMatlParams(costs);

  EXPECT_DOUBLE_EQ(4.0, bid->unit_cost());
  EXPECT_DOUBLE_EQ(6.0, costs[request][bid]);

  delete bid;
  delete request;
}

}  // namespace cycamore

cyclus::Agent* TariffRegionConstructor(cyclus::Context* ctx) {
  return new cycamore::TariffRegion(ctx);
}

#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

INSTANTIATE_TEST_SUITE_P(TariffRegion, RegionTests,
                        ::testing::Values(&TariffRegionConstructor));
INSTANTIATE_TEST_SUITE_P(TariffRegion, AgentTests,
                        ::testing::Values(&TariffRegionConstructor));
