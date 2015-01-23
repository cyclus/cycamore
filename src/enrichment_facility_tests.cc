#include <gtest/gtest.h>

#include <sstream>

#include "facility_tests.h"
#include "toolkit/mat_query.h"
#include "agent_tests.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "env.h"

#include "enrichment_facility_tests.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUp() {
  cyclus::Env::SetNucDataPath();
  cyclus::Context* ctx = tc_.get();
  src_facility = new EnrichmentFacility(ctx);
  trader = tc_.trader();
  InitParameters();
  SetUpSource();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::TearDown() {
  delete src_facility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::InitParameters() {
  cyclus::Context* ctx = tc_.get();

  in_commod = "incommod";
  out_commod = "outcommod";

  in_recipe = "recipe";
  feed_assay = 0.0072;

  cyclus::CompMap v;
  v[922350000] = feed_assay;
  v[922380000] = 1 - feed_assay;
  recipe = cyclus::Composition::CreateFromAtom(v);
  ctx->AddRecipe(in_recipe, recipe);

  tails_assay = 0.002;
  swu_capacity = 100;
  inv_size = 5;

  reserves = 105.5;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::SetUpSource() {
  src_facility->InRecipe(in_recipe);
  src_facility->in_commodity(in_commod);
  src_facility->out_commodity(out_commod);
  src_facility->TailsAssay(tails_assay);
  //  src_facility->FeedAssay(feed_assay);
  src_facility->SetMaxInventorySize(inv_size);
  src_facility->SwuCapacity(swu_capacity);
  src_facility->InitialReserves(reserves);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::GetMat(double qty) {
  return cyclus::Material::CreateUntracked(qty,
                                           tc_.get()->GetRecipe(in_recipe));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::GetReqMat(double qty,
                                                        double enr) {
  cyclus::CompMap v;
  v[922350000] = enr;
  v[922380000] = 1 - enr;
  return cyclus::Material::CreateUntracked(
      qty, cyclus::Composition::CreateFromAtom(v));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacilityTest::DoAddMat(cyclus::Material::Ptr mat) {
  src_facility->AddMat_(mat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacilityTest::DoRequest() {
  return src_facility->Request_();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr
EnrichmentFacilityTest::DoOffer(cyclus::Material::Ptr mat) {
  return src_facility->Offer_(mat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr
EnrichmentFacilityTest::DoEnrich(cyclus::Material::Ptr mat, double qty) {
  return src_facility->Enrich_(mat, qty);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, InitialState) {
  EXPECT_EQ(in_recipe, src_facility->InRecipe());
  EXPECT_EQ(in_commod, src_facility->in_commodity());
  EXPECT_EQ(out_commod, src_facility->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, src_facility->TailsAssay());
  //  EXPECT_DOUBLE_EQ(feed_assay, src_facility->FeedAssay());
  EXPECT_DOUBLE_EQ(inv_size, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, src_facility->SwuCapacity());
  EXPECT_EQ(reserves, src_facility->InitialReserves());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<config>"
     << "<UNSPECIFIED>"
     << "  <input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "    <inventorysize>" << inv_size << "</inventorysize>"
     << "  </input>"
     << "  <output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <tails_assay>" << tails_assay << "</tails_assay>"
     << "    <swu_capacity>" << swu_capacity << "</swu_capacity>"
     << "  </output>"
     << "  <initial_condition>"
     << "    <reserves_qty>" << reserves << "</reserves_qty>"
     << "  </initial_condition>"
     << "</UNSPECIFIED>"
     << "</config>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::EnrichmentFacility fac(tc_.get());

  // EXPECT_NO_THROW(fac.InitFrom(&engine););
  EXPECT_EQ(in_recipe, fac.InRecipe());
  EXPECT_EQ(in_commod, fac.in_commodity());
  EXPECT_EQ(out_commod, fac.out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, fac.TailsAssay());
  //  EXPECT_DOUBLE_EQ(feed_assay, fac.FeedAssay());
  EXPECT_DOUBLE_EQ(inv_size, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(0.0, fac.InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, fac.SwuCapacity());
  EXPECT_EQ(reserves, fac.InitialReserves());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Clone) {
  cyclus::Context* ctx = tc_.get();

  cycamore::EnrichmentFacility* cloned_fac =
    dynamic_cast<cycamore::EnrichmentFacility*>(src_facility->Clone());

  EXPECT_EQ(in_recipe, cloned_fac->InRecipe());
  EXPECT_EQ(in_commod, cloned_fac->in_commodity());
  EXPECT_EQ(out_commod, cloned_fac->out_commodity());
  EXPECT_DOUBLE_EQ(tails_assay, cloned_fac->TailsAssay());
  //  EXPECT_DOUBLE_EQ(feed_assay, cloned_fac->FeedAssay());
  EXPECT_DOUBLE_EQ(inv_size, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(swu_capacity, cloned_fac->SwuCapacity());
  EXPECT_EQ(reserves, cloned_fac->InitialReserves());

  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, AddMat) {
  EXPECT_THROW(DoAddMat(test_helpers::get_mat()), cyclus::ValueError);
  EXPECT_THROW(DoAddMat(GetMat(inv_size + 1)), cyclus::Error);
  EXPECT_NO_THROW(DoAddMat(GetMat(inv_size)));
  EXPECT_THROW(DoAddMat(GetMat(1)), cyclus::Error);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Request) {
  double req = inv_size;
  double add = 0;
  cyclus::Material::Ptr mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));

  add = 2 * inv_size / 3;
  req -= add;
  DoAddMat(GetMat(add));
  mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));

  add = inv_size / 3;
  req = 0;
  DoAddMat(GetMat(add));
  mat = DoRequest();
  EXPECT_DOUBLE_EQ(mat->quantity(), req);
  EXPECT_EQ(mat->comp(), tc_.get()->GetRecipe(in_recipe));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Offer) {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;
  using cyclus::toolkit::MatQuery;

  double qty = 4.5;
  double u234 = 1.0;
  double u235 = 1.0;
  double u238 = 2.0;
  cyclus::CompMap v;
  v[94239] = u234;
  v[922350000] = u235;
  v[922380000] = u238;
  Material::Ptr mat =
      DoOffer(Material::CreateUntracked(qty, Composition::CreateFromAtom(v)));

  MatQuery q(mat);

  EXPECT_DOUBLE_EQ(q.atom_frac(94239), 0.0);
  EXPECT_DOUBLE_EQ(q.atom_frac(922350000), u235 / (u235 + u238));
  EXPECT_DOUBLE_EQ(q.atom_frac(922380000), u238 / (u235 + u238));
  EXPECT_DOUBLE_EQ(mat->quantity(), qty);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, ValidReq) {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;

  double qty = 4.5;  // some magic number

  cyclus::CompMap v1;
  v1[922350000] = 1;
  Material::Ptr mat = Material::CreateUntracked(qty,
                                                Composition::CreateFromAtom(v1));
  EXPECT_TRUE(!src_facility->ValidReq(mat));  // u238 = 0

  cyclus::CompMap v2;
  v2[922350000] = tails_assay;
  v2[922380000] = 1 - tails_assay;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v2));
  EXPECT_TRUE(!src_facility->ValidReq(mat));  // u235 / (u235 + u238) <= tails_assay

  cyclus::CompMap v3;
  v3[922350000] = 1;
  v3[922380000] = 1;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v3));
  EXPECT_TRUE(src_facility->ValidReq(mat));  // valid
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, EmptyRequests) {
  using cyclus::Material;
  using cyclus::RequestPortfolio;

  src_facility->SetMaxInventorySize(src_facility->InventorySize());
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();
  ports = src_facility->GetMatlRequests();
  EXPECT_TRUE(ports.empty());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, AddRequests) {
  using cyclus::Request;
  using cyclus::RequestPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;

  // a request is made for the current available inventory amount

  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();

  ASSERT_EQ(ports.size(), 1);
  ASSERT_EQ(ports.begin()->get()->qty(), inv_size);

  const std::vector<Request<Material>*>& requests =
      ports.begin()->get()->requests();
  ASSERT_EQ(requests.size(), 1);

  Request<Material>* req = *requests.begin();
  EXPECT_EQ(req->requester(), src_facility);
  EXPECT_EQ(req->commodity(), in_commod);

  const std::set< CapacityConstraint<Material> >& constraints =
      ports.begin()->get()->constraints();
  EXPECT_EQ(constraints.size(), 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Extract) {
  using cyclus::Material;
  cyclus::Env::SetNucDataPath();
  double qty = 1000;  // 5 kg
  Material::Ptr base = GetMat(qty);
  double time = tc_.get()->time();
      // cyclus::Material::Create(src_facility, qty,
      //                                   tc_.get()->GetRecipe(in_recipe));
  Material::Ptr base2 = GetMat(qty);
  base->Absorb(base2);
  double product_assay = 0.05;  // of 5 w/o enriched U
  cyclus::CompMap v;
  v[922350000] = product_assay;
  v[922380000] = 1 - product_assay;
  // target qty need not be = to request qty
  Material::Ptr target = cyclus::Material::CreateUntracked(
      5, cyclus::Composition::CreateFromMass(v));
  Material::Ptr response = base->ExtractComp(6, target->comp());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Accept) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;

  // an enrichment facility gets two trades, each for 1/3 of its inv size
  // note that comp != recipe is covered by AddMat tests
  // note that qty >= inv capacity is covered by toolkit::ResourceBuff tests

  double qty = inv_size / 3;
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> > responses;

  Request<Material>* req1 =
      Request<Material>::Create(DoRequest(), src_facility, in_commod);
  Bid<Material>* bid1 =
      Bid<Material>::Create(req1, GetMat(qty), trader);

  Request<Material>* req2 =
      Request<Material>::Create(DoRequest(), src_facility, in_commod);
  Bid<Material>* bid2 = Bid<Material>::Create(req2, GetMat(qty), trader);

  Trade<Material> trade1(req1, bid1, qty);
  responses.push_back(std::make_pair(trade1, GetMat(qty)));
  Trade<Material> trade2(req2, bid2, qty);
  responses.push_back(std::make_pair(trade2, GetMat(qty)));

  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  src_facility->AcceptMatlTrades(responses);
  EXPECT_DOUBLE_EQ(qty * 2, src_facility->InventorySize());

  delete bid2;
  delete bid1;
  delete req2;
  delete req1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, AddBids) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::ExchangeContext;
  using cyclus::Material;
  cyclus::Env::SetNucDataPath();
  // an enrichment facility bids on nreqs requests
  // note that bid response is covered by Bid tests
  // note that validity of requests is covered by ValidReq tests
  int nreqs = 5;
  int nvalid = 4;

  // set up inventory
  double current_size = inv_size / 2;  // test something other than max size
  DoAddMat(GetMat(current_size));

  boost::shared_ptr< cyclus::ExchangeContext<Material> >
      ec = GetContext(nreqs, nvalid);

  std::set<BidPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlBids(ec.get()->commod_requests);

  ASSERT_TRUE(ports.size() > 0);
  EXPECT_EQ(ports.size(), 1);

  BidPortfolio<Material>::Ptr port = *ports.begin();
  EXPECT_EQ(port->bidder(), src_facility);
  EXPECT_EQ(port->bids().size(), nvalid);

  const std::set< CapacityConstraint<Material> >& constrs = port->constraints();
  Converter<Material>::Ptr sc(new SWUConverter(feed_assay, tails_assay));
  Converter<Material>::Ptr nc(new NatUConverter(feed_assay, tails_assay));
  CapacityConstraint<Material> swu(swu_capacity, sc);
  CapacityConstraint<Material> natu(current_size, nc);
  EXPECT_EQ(constrs.size(), 2);
  EXPECT_TRUE(*constrs.begin() == swu || *(++constrs.begin()) == swu);
  EXPECT_TRUE(*constrs.begin() == natu || *(++constrs.begin()) == natu);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
boost::shared_ptr< cyclus::ExchangeContext<cyclus::Material> >
EnrichmentFacilityTest::GetContext(int nreqs, int nvalid) {
  using cyclus::ExchangeContext;
  using cyclus::Material;
  using cyclus::Request;
  using test_helpers::get_mat;

  boost::shared_ptr< ExchangeContext<Material> >
      ec(new ExchangeContext<Material>());
  for (int i = 0; i < nvalid; i++) {
    ec->AddRequest(
        Request<Material>::Create(GetReqMat(1.0, 0.05), trader, out_commod));
  }
  for (int i = 0; i < nreqs - nvalid; i++) {
    ec->AddRequest(
        // get_mat returns a material of only u235, which is not valid
        Request<Material>::Create(get_mat(), trader, out_commod));
  }
  return ec;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, BidConverters) {
  // this test is designed to confirm that the bid response behavior matches the
  // converter behavior.
  using cyclus::CompMap;
  using cyclus::Material;
  using cyclus::toolkit::MatQuery;
  using cyclus::Composition;
  using cyclus::toolkit::Assays;
  using cyclus::toolkit::UraniumAssay;
  using cyclus::toolkit::SwuRequired;
  using cyclus::toolkit::FeedQty;
  cyclus::Env::SetNucDataPath();

  double qty = 5;  // 5 kg
  double product_assay = 0.05;  // of 5 w/o enriched U
  CompMap v;
  v[922350000] = product_assay;
  v[922380000] = 1 - product_assay;
  v[94239] = 0.5;  // 94239 shouldn't be taken into account
  Material::Ptr target = Material::CreateUntracked(
      qty, Composition::CreateFromMass(v));

  SWUConverter swuc(feed_assay, tails_assay);
  NatUConverter natuc(feed_assay, tails_assay);

  Material::Ptr offer = DoOffer(target);

  EXPECT_NEAR(swuc.convert(target), swuc.convert(offer), 0.001);
  EXPECT_NEAR(natuc.convert(target), natuc.convert(offer), 0.001);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Enrich) {
  // this test asks the facility to enrich a material that results in an amount
  // of natural uranium required that is exactly its inventory level. that
  // inventory will be comprised of two materials to test the manifest/absorb
  // strategy employed in Enrich_.
  using cyclus::CompMap;
  using cyclus::Material;
  using cyclus::toolkit::MatQuery;
  using cyclus::Composition;
  using cyclus::toolkit::Assays;
  using cyclus::toolkit::UraniumAssay;
  using cyclus::toolkit::SwuRequired;
  using cyclus::toolkit::FeedQty;

  double qty = 5;  // 5 kg
  double product_assay = 0.05;  // of 5 w/o enriched U
  cyclus::CompMap v;
  v[922350000] = product_assay;
  v[922380000] = 1 - product_assay;
  // target qty need not be = to request qty
  Material::Ptr target = cyclus::Material::CreateUntracked(
      qty + 10, cyclus::Composition::CreateFromMass(v));

  Assays assays(feed_assay, UraniumAssay(target), tails_assay);
  double swu_req = SwuRequired(qty, assays);
  double natu_req = FeedQty(qty, assays);
  double tails_qty = TailsQty(qty, assays);

  double swu_cap = swu_req * 5;
  src_facility->SwuCapacity(swu_cap);
  src_facility->SetMaxInventorySize(natu_req);
  DoAddMat(GetMat(natu_req / 2));
  DoAddMat(GetMat(natu_req / 2));

  Material::Ptr response;
  EXPECT_NO_THROW(response = DoEnrich(target, qty));
  EXPECT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(), swu_cap - swu_req);
  EXPECT_DOUBLE_EQ(src_facility->Tails().quantity(), tails_qty);
  
  MatQuery q(response);
  EXPECT_EQ(response->quantity(), qty);
  EXPECT_EQ(q.mass_frac(922350000), product_assay);
  EXPECT_EQ(q.mass_frac(922380000), 1 - product_assay);

  // test too much natu request
  DoAddMat(GetMat(natu_req - 1));
  EXPECT_THROW(response = DoEnrich(target, qty), cyclus::Error);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, Response) {
  // this test asks the facility to respond to multiple requests for enriched
  // uranium. two requests are provided, whose total equals the swu capacity of
  // the facility while not exceeding its inventory capacity (that's taken care
  // of in the Enrich tests).
  //
  // note that response quantity and quality need not be tested, because they
  // are covered by the Enrich and Offer tests
  using cyclus::Bid;
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using cyclus::toolkit::MatQuery;
  using cyclus::toolkit::Assays;
  using cyclus::toolkit::FeedQty;
  using cyclus::toolkit::SwuRequired;
  using cyclus::toolkit::UraniumAssay;
  using test_helpers::get_mat;

  // problem set up
  std::vector< cyclus::Trade<cyclus::Material> > trades;
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> > responses;

  double qty = 5;  // 5 kg
  double trade_qty = qty / 3;
  double product_assay = 0.05;  // of 5 w/o enriched U
  cyclus::CompMap v;
  v[922350000] = product_assay;
  v[922380000] = 1 - product_assay;
  // target qty need not be = to request qty
  Material::Ptr target = cyclus::Material::CreateUntracked(
      qty + 10, cyclus::Composition::CreateFromMass(v));

  Assays assays(feed_assay, UraniumAssay(target), tails_assay);
  double swu_req = SwuRequired(qty, assays);
  double natu_req = FeedQty(qty, assays);

  src_facility->SetMaxInventorySize(natu_req * 4);  // not capacitated by nat u
  src_facility->SwuCapacity(swu_req);  // swu capacitated

  // Null response
  src_facility->GetMatlTrades(trades, responses);
  EXPECT_NO_THROW();
  EXPECT_EQ(responses.size(), 0);

  // set up state
  DoAddMat(GetMat(natu_req * 2));

  Request<Material>* req =
      Request<Material>::Create(target, trader, out_commod);
  Bid<Material>* bid = Bid<Material>::Create(req, target, src_facility);
  Trade<Material> trade(req, bid, trade_qty);
  trades.push_back(trade);

  // 1 trade, SWU < SWU cap
  ASSERT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(), swu_req);
  src_facility->GetMatlTrades(trades, responses);
  ASSERT_EQ(responses.size(), 1);
  EXPECT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(),
                   swu_req - SwuRequired(trade_qty, assays));

  // 2 trades, SWU = SWU cap
  ASSERT_GT(src_facility->CurrentSwuCapacity() - 2 * swu_req / 3,
            -1 * cyclus::eps());
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);
  EXPECT_TRUE(cyclus::AlmostEq(src_facility->CurrentSwuCapacity(), 0));

  // too much qty, capn!
  trade = Trade<Material>(req, bid, 1);  // a small number
  trades.clear();
  trades.push_back(trade);
  EXPECT_THROW(src_facility->GetMatlTrades(trades, responses),
               cyclus::ValueError);

  // reset!
  src_facility->Tick();
  EXPECT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(), swu_req);
}

}  // namespace cycamore

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* EnrichmentFacilityConstructor(cyclus::Context* ctx) {
  return new cycamore::EnrichmentFacility(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(EnrichmentFac, FacilityTests,
                        Values(&EnrichmentFacilityConstructor));
INSTANTIATE_TEST_CASE_P(EnrichmentFac, AgentTests,
                        Values(&EnrichmentFacilityConstructor));
