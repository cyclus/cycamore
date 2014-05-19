// source_facility_tests.cc
#include <gtest/gtest.h>

#include <sstream>

#include "cyc_limits.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "xml_parser.h"

#include "source_facility_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::SetUp() {
  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::TearDown() {
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::InitParameters() {
  commod = "commod";
  recipe_name = "recipe";
  capacity = 5; // some magic number..

  recipe = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  tc.get()->AddRecipe(recipe_name, recipe);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::SetUpSourceFacility() {
  src_facility = new cycamore::SourceFacility(tc.get());
  src_facility->commodity(commod);
  src_facility->recipe(recipe_name);
  src_facility->capacity(capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, InitialState) {
  EXPECT_EQ(src_facility->capacity(), capacity);
  EXPECT_EQ(src_facility->commodity(), commod);
  EXPECT_EQ(src_facility->recipe(), recipe_name);
  EXPECT_EQ(src_facility->current_capacity(), capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<agent>"
     << "<UNSPECIFIED>"
     << "<output>"
     << "  <outcommodity>" << commod << "</outcommodity>"
     << "  <output_capacity>" << capacity << "</output_capacity>"
     << "  <recipe>" << recipe_name << "</recipe>"
     << "</output>"
     << "</UNSPECIFIED>"
     << "</agent>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::SourceFacility fac(tc.get());

  //EXPECT_NO_THROW(fac.InitFrom(&engine););
  EXPECT_EQ(fac.capacity(), capacity);
  EXPECT_EQ(fac.commodity(), commod);
  EXPECT_EQ(fac.recipe(), recipe_name);
  EXPECT_EQ(fac.current_capacity(), capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Clone) {
  cyclus::Context* ctx = tc.get();
  cycamore::SourceFacility* cloned_fac = dynamic_cast<cycamore::SourceFacility*>
                                         (src_facility->Clone());

  EXPECT_EQ(src_facility->commodity(), cloned_fac->commodity());
  EXPECT_EQ(src_facility->capacity(), cloned_fac->capacity());
  EXPECT_EQ(src_facility->recipe(), cloned_fac->recipe());
  EXPECT_EQ(src_facility->capacity(), cloned_fac->current_capacity());

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, GetOffer) {
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
TEST_F(SourceFacilityTest, AddBids) {
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
TEST_F(SourceFacilityTest, Response) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::trader;
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
  ASSERT_EQ(src_facility->current_capacity(), capacity);
  src_facility->GetMatlTrades(trades, responses);
  EXPECT_EQ(responses.size(), 1);
  EXPECT_EQ(responses[0].second->quantity(), qty);
  EXPECT_EQ(responses[0].second->comp(), recipe);

  // 2 trades, total qty = capacity
  ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity - qty);
  ASSERT_GT(src_facility->current_capacity() - 2 * qty, -1 * cyclus::eps());
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);
  ASSERT_TRUE(cyclus::AlmostEq(src_facility->current_capacity(), 0));

  // too much qty, capn!
  EXPECT_THROW(src_facility->GetMatlTrades(trades, responses),
               cyclus::ValueError);
  
  // reset!
  src_facility->Tick(1);
  ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
SourceFacilityTest::GetContext(int nreqs, std::string commod) {
  double qty = 3;
  boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
                     ec(new cyclus::ExchangeContext<cyclus::Material>());
  for (int i = 0; i < nreqs; i++) {
    using cyclus::Material;
    using cyclus::Request;
    using test_helpers::trader;
    using test_helpers::get_mat;
    ec->AddRequest(Request<Material>::Create(get_mat(), trader, commod));
  }
  return ec;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SourceFacilityConstructor(cyclus::Context* ctx) {
  return new cycamore::SourceFacility(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SourceFac, FacilityTests,
                        Values(&SourceFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, AgentTests,
                        Values(&SourceFacilityConstructor));
