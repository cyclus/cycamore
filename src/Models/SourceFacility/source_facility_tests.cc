// source_facility_tests.cc
#include <gtest/gtest.h>

#include <sstream>

#include "resource_helpers.h"
#include "xml_query_engine.h"
#include "xml_parser.h"

#include "source_facility_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::SetUp() {
  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::TearDown() {
  delete src_facility;
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
  src_facility->set_commodity(commod);
  src_facility->set_recipe(recipe_name);
  src_facility->set_capacity(capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Init) {
  std::stringstream ss;
  ss << "<root>" << "<output>"
     << "<outcommodity>" << commod << "</outcommodity>"
     << "<output_capacity>" << capacity << "</output_capacity>"
     << "<recipe>" << recipe_name << "</recipe>"
     << "</output>" << "</root>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::XMLQueryEngine engine(p);
  cycamore::SourceFacility fac(tc.get());

  EXPECT_NO_THROW(fac.InitModuleMembers(&engine););
  EXPECT_EQ(fac.capacity(), capacity);
  EXPECT_EQ(fac.commodity(), commod);
  EXPECT_EQ(fac.recipe(), recipe_name);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Clone) {
  cyclus::Context* ctx = tc.get();
  cycamore::SourceFacility* cloned_fac = dynamic_cast<cycamore::SourceFacility*>
                                         (src_facility->Clone());

  EXPECT_EQ(src_facility->commodity(), cloned_fac->commodity());
  EXPECT_EQ(src_facility->capacity(), cloned_fac->capacity());
  EXPECT_EQ(src_facility->recipe(), cloned_fac->recipe());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Offer) {
  using cyclus::Material;
  
  double qty = capacity - 1;
  Material::Ptr mat = Material::CreateBlank(qty);
  Material::Ptr obs_mat = src_facility->__GetOffer(mat);
  EXPECT_EQ(obs_mat->quantity(), qty);
  EXPECT_EQ(obs_mat->comp(), recipe);
  
  qty = capacity + 1;
  mat = Material::CreateBlank(qty);
  obs_mat = src_facility->__GetOffer(mat);
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
  using cyclus::ExchangeContext;
  
  int nreqs = 5;
  
  boost::shared_ptr< cyclus::ExchangeContext<Material> >
      ec = GetContext(nreqs, commod);
  
  std::set<BidPortfolio<Material>::Ptr> ports =
      src_facility->AddMatlBids(ec.get());

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
    ec->__AddRequest(Request<Material>::Ptr(
        new Request<Material>(get_mat(), &trader, commod)));
  }
  return ec;
}

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TEST_F(SourceFacilityTest, Tock) {
//   int time = 1;
//   EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
//   EXPECT_NO_THROW(src_facility->HandleTock(time));
// }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::SourceFacility;
  return dynamic_cast<cyclus::Model*>(new SourceFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SourceFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::SourceFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new SourceFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SourceFac, FacilityModelTests,
                        Values(&SourceFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, ModelTests,
                        Values(&SourceFacilityModelConstructor));
