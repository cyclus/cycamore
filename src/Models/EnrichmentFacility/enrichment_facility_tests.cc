// enrichment_facility_tests.cc
#include <gtest/gtest.h>

#include <sstream>

#include "commodity.h"
#include "facility_model_tests.h"
#include "model_tests.h"
#include "resource_helpers.h"
#include "xml_query_engine.h"

#include "enrichment_facility_tests.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUp() {
  cyclus::Context* ctx = tc_.get();
  src_facility = new EnrichmentFacility(ctx);

  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::InitParameters() {
  cyclus::Context* ctx = tc_.get();

  in_commod = "incommod";
  out_commod = "outcommod";

  in_recipe = "recipe";
  feed_assay = 0.0072;

  cyclus::CompMap v;
  v[92235] = feed_assay;
  v[92238] = 1 - feed_assay;
  recipe = cyclus::Composition::CreateFromAtom(v);
  ctx->AddRecipe(in_recipe, recipe);

  tails_assay = 0.002;
  inv_size = 5;
  commodity_price = 0;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUpSourceFacility() {
  src_facility->in_recipe(in_recipe);
  src_facility->in_commodity(in_commod);
  src_facility->out_commodity(out_commod);
  src_facility->tails_assay(tails_assay);
  src_facility->feed_assay(feed_assay);
  src_facility->commodity_price(commodity_price);
  src_facility->SetMaxInventorySize(inv_size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::GetMat(double qty) {
  return cyclus::Material::CreateUntracked(qty,
                                           tc_.get()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::DoAbsorb(cyclus::Material::Ptr mat) {
  src_facility->Absorb_(mat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, InitialState) {
  EXPECT_EQ(in_recipe, src_facility->in_recipe());
  EXPECT_EQ(in_commod, src_facility->in_commodity());
  EXPECT_EQ(out_commod, src_facility->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, src_facility->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, src_facility->feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, src_facility->commodity_price());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventoryQty());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "  <input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "    <inventorysize>" << inv_size << "</inventorysize>"
     << "  </input>"
     << "  <output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <tails_assay>" << tails_assay << "</tails_assay>"
     << "  </output>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::XMLQueryEngine engine(p);
  cycamore::EnrichmentFacility fac(tc_.get());

  EXPECT_NO_THROW(fac.InitModuleMembers(&engine););
  EXPECT_EQ(in_recipe, fac.in_recipe());
  EXPECT_EQ(in_commod, fac.in_commodity());
  EXPECT_EQ(out_commod, fac.out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, fac.tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, fac.feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, fac.commodity_price());
  EXPECT_DOUBLE_EQ(0.0, fac.InventoryQty());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Clone) {
  cyclus::Context* ctx = tc_.get();

  cycamore::EnrichmentFacility* cloned_fac =
    dynamic_cast<cycamore::EnrichmentFacility*>(src_facility->Clone());

  EXPECT_EQ(in_recipe, cloned_fac->in_recipe());
  EXPECT_EQ(in_commod, cloned_fac->in_commodity());
  EXPECT_EQ(out_commod, cloned_fac->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, cloned_fac->tails_assay());
  EXPECT_DOUBLE_EQ(feed_assay, cloned_fac->feed_assay());
  EXPECT_DOUBLE_EQ(inv_size, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(commodity_price, cloned_fac->commodity_price());
  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventoryQty());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Absorb) {
  EXPECT_THROW(DoAbsorb(test_helpers::get_mat()), cyclus::StateError);
  EXPECT_THROW(DoAbsorb(GetMat(inv_size + 1)), cyclus::ValueError);
  EXPECT_NO_THROW(DoAbsorb(GetMat(inv_size)));
  EXPECT_THROW(DoAbsorb(GetMat(1)), cyclus::ValueError);
}

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TEST_F(SourceFacilityTest, Response) {
//   using cyclus::Bid;
//   using cyclus::Material;
//   using cyclus::Request;
//   using cyclus::Trade;
//   using test_helpers::trader;
//   using test_helpers::get_mat;

//   std::vector< cyclus::Trade<cyclus::Material> > trades;
//   std::vector<std::pair<cyclus::Trade<cyclus::Material>,
//                         cyclus::Material::Ptr> > responses;

//   // Null response
//   EXPECT_NO_THROW(src_facility->PopulateMatlTradeResponses(trades, responses));
//   EXPECT_EQ(responses.size(), 0);

//   double qty = capacity / 3;
//   Request<Material>::Ptr request(
//       new Request<Material>(get_mat(), &trader, commod));
//   Bid<Material>::Ptr bid(new Bid<Material>(request, get_mat(), src_facility));

//   Trade<Material> trade(request, bid, qty);
//   trades.push_back(trade);

//   // 1 trade, SWU < SWU cap, NatU < NatU cap
//   ASSERT_EQ(src_facility->current_capacity(), capacity);
//   src_facility->PopulateMatlTradeResponses(trades, responses);
//   EXPECT_EQ(responses.size(), 1);
//   EXPECT_EQ(responses[0].second->quantity(), qty);
//   EXPECT_EQ(responses[0].second->comp(), recipe);

//   // 1 trade, SWU > SWU Cap, NatU < NatU cap
//   ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity - qty);
//   ASSERT_GT(src_facility->current_capacity() - 2 * qty, -1 * cyclus::eps());
//   trades.push_back(trade);
//   responses.clear();
//   EXPECT_NO_THROW(src_facility->PopulateMatlTradeResponses(trades, responses));
//   EXPECT_EQ(responses.size(), 2);
//   ASSERT_TRUE(cyclus::AlmostEq(src_facility->current_capacity(), 0));

//   // 1 trade, SWU < SWU Cap, NatU > NatU cap
//   // 1 trade, SWU = SWU Cap, NatU < NatU cap
//   // 1 trade, SWU < SWU Cap, NatU = NatU cap
  
//   // // too much qty, capn!
//   // EXPECT_THROW(src_facility->PopulateMatlTradeResponses(trades, responses),
//   //              cyclus::StateError);
  
//   // // reset!
//   // src_facility->HandleTick(1);
//   // ASSERT_DOUBLE_EQ(src_facility->current_capacity(), capacity);
// }

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
// SourceFacilityTest::GetContext(int nreqs, std::string commod) {
//   double qty = 3;
//   boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
//                      ec(new cyclus::ExchangeContext<cyclus::Material>());
//   for (int i = 0; i < nreqs; i++) {
//     using cyclus::Material;
//     using cyclus::Request;
//     using test_helpers::trader;
//     using test_helpers::get_mat;
//     ec->AddRequest(Request<Material>::Ptr(
//         new Request<Material>(get_mat(), &trader, commod)));
//   }
//   return ec;
// }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* EnrichmentFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::EnrichmentFacility;
  return dynamic_cast<cyclus::Model*>(new EnrichmentFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* EnrichmentFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::EnrichmentFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new EnrichmentFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(EnrichmentFac, FacilityModelTests,
                        Values(&EnrichmentFacilityConstructor));
INSTANTIATE_TEST_CASE_P(EnrichmentFac, ModelTests,
                        Values(&EnrichmentFacilityModelConstructor));

} // namespace cycamore
