#include <gtest/gtest.h>

#include <sstream>

#include "facility_tests.h"
#include "toolkit/mat_query.h"
#include "agent_tests.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "env.h"

#include "enrichment_facility_tests.h"

using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::CompMap;
using cyclus::toolkit::MatQuery;
using pyne::nucname::id;
using cyclus::Material;

namespace cycamore {
  
Composition::Ptr c_nou235() {
  cyclus::CompMap m;
  m[922380000] = 1.0;
  return Composition::CreateFromMass(m);
};
Composition::Ptr c_natu1() {
  cyclus::CompMap m;
  m[922350000] = 0.007;
  m[922380000] = 0.993;
  return Composition::CreateFromMass(m);
};
Composition::Ptr c_natu2() {
  cyclus::CompMap m;
  m[922350000] = 0.01;
  m[922380000] = 0.99;
  return Composition::CreateFromMass(m);
};
Composition::Ptr c_leu() {
  cyclus::CompMap m;
  m[922350000] = 0.04;
  m[922380000] = 0.96;
  return Composition::CreateFromMass(m);
};
Composition::Ptr c_heu() {
  cyclus::CompMap m;
  m[922350000] = 0.20;
  m[922380000] = 0.80;
  return Composition::CreateFromMass(m);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, RequestEnrich) {
  // this tests verifies that requests for material exceeding the maximum
  // allowed enrichment are not fulfilled.
  
  std::string config = 
    "   <in_commod>natu</in_commod> "
    "   <in_recipe>natu1</in_recipe> "
    "   <out_commod>enr_u</out_commod> "
    "   <tails_commod>tails</tails_commod> "
    "   <tails_assay>0.003</tails_assay> "
    "   <max_enrich>0.20</max_enrich> " ;

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec
		      (":cycamore:EnrichmentFacility"), config, simdur);
  sim.AddRecipe("natu1", c_natu1()) ;
  sim.AddRecipe("leu", c_leu()) ;
  sim.AddRecipe("heu", c_heu()) ;
  
  sim.AddSource("natu")
    .recipe("natu1")
    .Finalize();
  sim.AddSink("enr_u")
    .recipe("leu")
    .Finalize();
  sim.AddSink("enr_u")
    .recipe("heu")
    .Finalize();
  
  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==",std::string("enr_u")));
  QueryResult qr = sim.db().Query("Transactions", &conds);

  // Should be only one transaction out of the EF,
  // and it should be LEU
  EXPECT_EQ(1.0, qr.rows.size());

  Material::Ptr m = sim.GetMaterial(qr.GetVal<int>("ResourceId"));
  CompMap got = m->comp()->mass();
  CompMap want = c_leu()->mass();
  cyclus::compmath::Normalize(&got);
  cyclus::compmath::Normalize(&want);

  CompMap::iterator it;
  for (it = want.begin(); it != want.end(); ++it) {
    EXPECT_DOUBLE_EQ(it->second, got[it->first]) <<
      "nuclide qty off: " << pyne::nucname::name(it->first);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, TradeTails) {
  // this tests whether tails are being traded.

  std::string config = 
    "   <in_commod>natu</in_commod> "
    "   <in_recipe>natu1</in_recipe> "
    "   <out_commod>enr_u</out_commod> "
    "   <tails_commod>tails</tails_commod> "
    "   <tails_assay>0.003</tails_assay> " ;

  //QQ - should tails buffer be available for trades on timestep 2?
  int simdur = 3 ;// 1-source to EF, 2-EF process, 3-add to tails inventory
  cyclus::MockSim sim(cyclus::AgentSpec
		      (":cycamore:EnrichmentFacility"), config, simdur);
  sim.AddRecipe("natu1", c_natu1()) ;
  sim.AddRecipe("leu", c_leu()) ;
  
  sim.AddSource("natu")
    .recipe("natu1")
    .Finalize();
  sim.AddSink("enr_u")
    .recipe("leu")
    .Finalize();
   sim.AddSink("tails")
    .Finalize();
  
  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==",std::string("tails")));
  QueryResult qr = sim.db().Query("Transactions", &conds);

  // Should be exactly one tails transaction
  EXPECT_EQ(1, qr.rows.size());
  
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, BidPrefs) {
  // This tests that natu sources are preference-ordered by
  // U235 content

  std::string config = 
    "   <in_commod>natu</in_commod> "
    "   <in_recipe>natu1</in_recipe> "
    "   <out_commod>enr_u</out_commod> "
    "   <tails_commod>tails</tails_commod> "
    "   <tails_assay>0.003</tails_assay> "
    "   <max_inv_size>1.0</max_inv_size> " ;

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
		      (":cycamore:EnrichmentFacility"), config, simdur);
  sim.AddRecipe("natu1", c_natu1()) ;
  sim.AddRecipe("natu2", c_natu2()) ;

  sim.AddSource("natu")
    .recipe("natu1")
    .capacity(1)
    .Finalize();

  sim.AddSource("natu")
    .recipe("natu2")
    .capacity(1)
    .Finalize();
  
  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==",std::string("natu")));
  QueryResult qr = sim.db().Query("Transactions", &conds);

  // should trade only with #2 since it has higher U235
  EXPECT_EQ(1, qr.rows.size());
  
  Material::Ptr m = sim.GetMaterial(qr.GetVal<int>("ResourceId"));
  CompMap got = m->comp()->mass();
  CompMap want = c_natu2()->mass();
  cyclus::compmath::Normalize(&got);
  cyclus::compmath::Normalize(&want);

  CompMap::iterator it;
  for (it = want.begin(); it != want.end(); ++it) {
    EXPECT_DOUBLE_EQ(it->second, got[it->first]) <<
      "nuclide qty off: " << pyne::nucname::name(it->first);
  }
  
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, NoBidPrefs) {
  // This tests that preference-ordering for sources
  // turns off correctly if flag is used

  std::string config = 
    "   <in_commod>natu</in_commod> "
    "   <in_recipe>natu1</in_recipe> "
    "   <out_commod>enr_u</out_commod> "
    "   <tails_commod>tails</tails_commod> "
    "   <tails_assay>0.003</tails_assay> "
    "   <max_inv_size>2.0</max_inv_size> " ;
    "   <order_prefs>0</order_prefs> " ;

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
		      (":cycamore:EnrichmentFacility"), config, simdur);
  sim.AddRecipe("natu1", c_natu1()) ;
  sim.AddRecipe("natu2", c_natu2()) ;

  sim.AddSource("natu")
    .recipe("natu1")
    .capacity(1)
    .Finalize();

  sim.AddSource("natu")
    .recipe("natu2")
    .capacity(1)
    .Finalize();
  
  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==",std::string("natu")));
  QueryResult qr = sim.db().Query("Transactions", &conds);

  // should trade with both to meet its capacity limit
  EXPECT_EQ(2, qr.rows.size());
  }
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(EnrichmentFacilityTest, ZeroU235) {
  // Test that offers of natu with no u235 content are rejected

  std::string config = 
    "   <in_commod>natu</in_commod> "
    "   <in_recipe>natu1</in_recipe> "
    "   <out_commod>enr_u</out_commod> "
    "   <tails_commod>tails</tails_commod> "
    "   <tails_assay>0.003</tails_assay> "
    "   <max_inv_size>1.0</max_inv_size> " ;

  int simdur = 1;
  cyclus::MockSim sim(cyclus::AgentSpec
		      (":cycamore:EnrichmentFacility"), config, simdur);
  sim.AddRecipe("no_u235", c_nou235()) ;
  sim.AddRecipe("natu1", c_natu1()) ;

  sim.AddSource("natu")
    .recipe("no_u235")
    .capacity(1)
    .Finalize();

  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("Commodity", "==",std::string("natu")));
  // DB table should be empty since there are no transactions
  EXPECT_THROW(sim.db().Query("Transactions", &conds),
	       std::exception);
}

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
  tails_commod = "tailscommod";

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
  src_facility->tails_commodity(tails_commod);
  src_facility->TailsAssay(tails_assay);
  src_facility->MaxEnrich(max_enrich);
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
// QQ Document, check that Fn is false when expected.
TEST_F(EnrichmentFacilityTest, ValidReq) {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;

  double qty = 4.5;  // some magic number

  cyclus::CompMap v1;
  v1[922350000] = 1;
  Material::Ptr mat = Material::CreateUntracked(qty,
                                                Composition::CreateFromAtom(v1));
  EXPECT_FALSE(src_facility->ValidReq(mat));  // u238 = 0

  cyclus::CompMap v2;
  v2[922350000] = tails_assay;
  v2[922380000] = 1 - tails_assay;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v2));
  EXPECT_FALSE(src_facility->ValidReq(mat));  // u235 / (u235 + u238) <= tails_assay

  cyclus::CompMap v3;
  v3[922350000] = 1;
  v3[922380000] = 1;
  mat = Material::CreateUntracked(qty, Composition::CreateFromAtom(v3));
  EXPECT_TRUE(src_facility->ValidReq(mat));  // valid
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// QQ Replace w/MockSim: generates requests with the correct amnt of material
// (requests to fill its inventory size - no more and no less)
// Generate case where source can generate more than inventory, amount matched is reasonable.
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


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Tests that SWU and invent. constraints are being properly applied.
  // MockSim that is SWU constrained - check amnt of matl traded.
  // Does traded amount meet SWU constraint? (1/2 qty of requested value)
  // Cap constraint - requests too much - supplies only amt avail.
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
  //QQ src_facility failing b/c of GetMat->AddMat_ ?
    EXPECT_EQ(port->bidder(), src_facility);

  //QQ bids().size=0, but nvalid=4  -->src_facility isn't working?
    EXPECT_EQ(port->bids().size(), nvalid);

  const std::set< CapacityConstraint<Material> >& constrs = port->constraints();
  Converter<Material>::Ptr sc(new SWUConverter(feed_assay, tails_assay));  //QQ
  Converter<Material>::Ptr nc(new NatUConverter(feed_assay, tails_assay)); //QQ
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
         Request<Material>::Create(get_mat(), trader, out_commod));
  }
  return ec;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Add documentation to clarify
  TEST_F(EnrichmentFacilityTest, ConstraintConverters) {
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
  using cyclus::toolkit::MatQuery;
  cyclus::Env::SetNucDataPath();

  double qty = 5;  // 5 kg
  double product_assay = 0.05;  // of 5 w/o enriched U
  CompMap v;
  v[922350000] = product_assay;
  v[922380000] = 1 - product_assay;
  v[94239] = 0.5;  // 94239 shouldn't be taken into account
  Material::Ptr target = Material::CreateUntracked(
      qty, Composition::CreateFromMass(v));

  std::set<cyclus::Nuc> nucs ;
  nucs.insert(922350000);
  nucs.insert(922380000);

  MatQuery mq(target) ;
  double mass_frac = mq.multi_mass_frac(nucs);
  
  SWUConverter swuc(feed_assay, tails_assay);
  NatUConverter natuc(feed_assay, tails_assay);

  Material::Ptr offer = DoOffer(target);

  EXPECT_NEAR(swuc.convert(target), swuc.convert(offer), 0.001);
  EXPECT_NEAR(natuc.convert(target)*mass_frac, natuc.convert(offer), 0.001); //QQ fixed
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// QQ KEEP HERE
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

  // 1 allowed trade, SWU < SWU cap
  ASSERT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(), swu_req);
  src_facility->GetMatlTrades(trades, responses);
  ASSERT_EQ(responses.size(), 1);  
  EXPECT_DOUBLE_EQ(src_facility->CurrentSwuCapacity(),
                   swu_req - SwuRequired(trade_qty, assays));

  // KEEP ONLY THIS SET OF TESTS
  // 2 trades, SWU = SWU cap
  ASSERT_GT(src_facility->CurrentSwuCapacity() - 2 * swu_req / 3,
            -1 * cyclus::eps());
  trades.push_back(trade);
  responses.clear();
  EXPECT_NO_THROW(src_facility->GetMatlTrades(trades, responses));
  EXPECT_EQ(responses.size(), 2);
  EXPECT_TRUE(cyclus::AlmostEq(src_facility->CurrentSwuCapacity(), 0));
  // END
  
  // too much qty, capn!
  trade = Trade<Material>(req, bid, 1);  // a small number
  trades.clear();
  trades.push_back(trade);
  EXPECT_THROW(src_facility->GetMatlTrades(trades, responses),
               cyclus::ValueError);
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
