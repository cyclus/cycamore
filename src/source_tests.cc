// source_tests.cc
#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "test_context.h"

#include "source_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceTest::SetUp() {
  src_facility = new cycamore::Source(tc.get());
  trader = tc.trader();
  InitParameters();
  SetUpSource();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceTest::InitParameters() {
  commod = "commod";
  recipe_name = "recipe";
  capacity = 5; // some magic number..

  recipe = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  tc.get()->AddRecipe(recipe_name, recipe);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceTest::SetUpSource() {
  src_facility->commodity(commod);
  src_facility->recipe(recipe_name);
  src_facility->Capacity(capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, InitialState) {
  EXPECT_EQ(src_facility->Capacity(), capacity);
  EXPECT_EQ(src_facility->commodity(), commod);
  EXPECT_EQ(src_facility->recipe(), recipe_name);
  EXPECT_EQ(src_facility->CurrentCapacity(), capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, Clone) {
  cyclus::Context* ctx = tc.get();
  cycamore::Source* cloned_fac = dynamic_cast<cycamore::Source*>
                                         (src_facility->Clone());

  EXPECT_EQ(src_facility->commodity(), cloned_fac->commodity());
  EXPECT_EQ(src_facility->Capacity(), cloned_fac->Capacity());
  EXPECT_EQ(src_facility->recipe(), cloned_fac->recipe());
  EXPECT_EQ(src_facility->Capacity(), cloned_fac->CurrentCapacity());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceTest, GetOffer) {
  using cyclus::Material;
  
  double qty = capacity - 1;
  Material::Ptr mat = cyclus::NewBlankMaterial(qty);
  Material::Ptr obs_mat = src_facility->GetOffer(mat);
  EXPECT_EQ(obs_mat->quantity(), qty);
  EXPECT_EQ(obs_mat->comp(), recipe);
  
  qty = capacity + 1;
  mat = cyclus::NewBlankMaterial(qty);
  obs_mat = src_facility->GetOffer(mat);
  EXPECT_EQ(obs_mat->quantity(), capacity);
  EXPECT_EQ(obs_mat->comp(), recipe);

  qty = capacity;
  mat = cyclus::NewBlankMaterial(qty);
  obs_mat = src_facility->GetOffer(mat);
  EXPECT_EQ(obs_mat->quantity(), capacity);
  EXPECT_EQ(obs_mat->comp(), recipe);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
  ASSERT_EQ(src_facility->CurrentCapacity(), capacity);
  src_facility->GetMatlTrades(trades, responses);
  EXPECT_EQ(responses.size(), 1);
  EXPECT_EQ(responses[0].second->quantity(), qty);
  EXPECT_EQ(responses[0].second->comp(), recipe);

  // 2 trades, total qty = capacity
  ASSERT_DOUBLE_EQ(src_facility->CurrentCapacity(), capacity - qty);
  ASSERT_GT(src_facility->CurrentCapacity() - 2 * qty, -1 * cyclus::eps());
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);
  ASSERT_TRUE(cyclus::AlmostEq(src_facility->CurrentCapacity(), 0));

  // too much qty, capn!
  EXPECT_THROW(src_facility->GetMatlTrades(trades, responses),
               cyclus::ValueError);
  
  // reset!
  src_facility->Tick(1);
  ASSERT_DOUBLE_EQ(src_facility->CurrentCapacity(), capacity);

  delete request;
  delete bid;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SourceConstructor(cyclus::Context* ctx) {
  return new cycamore::Source(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SourceFac, FacilityTests,
                        Values(&SourceConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, AgentTests,
                        Values(&SourceConstructor));
