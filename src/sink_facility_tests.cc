// sink_facility_tests.cc
#include <gtest/gtest.h>

#include "facility_tests.h"
#include "agent_tests.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "xml_parser.h"

#include "sink_facility_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::SetUp() {
  src_facility = new cycamore::SinkFacility(tc_.get());
  trader = tc_.trader();
  InitParameters();
  SetUpSinkFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::InitParameters() {
  commod1_ = "acommod";
  commod2_ = "bcommod";
  commod3_ = "ccommod";
  capacity_ = 5;
  inv_ = capacity_ * 2;
  qty_ = capacity_ * 0.5;
  ncommods_ = 2;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::SetUpSinkFacility() {
  src_facility->AddCommodity(commod1_);
  src_facility->AddCommodity(commod2_);
  src_facility->capacity(capacity_);
  src_facility->SetMaxInventorySize(inv_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, InitialState) {
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->capacity());
  EXPECT_DOUBLE_EQ(inv_, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, src_facility->input_commodities());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Clone) {
  using cycamore::SinkFacility;
  SinkFacility* cloned_fac = dynamic_cast<cycamore::SinkFacility*>
                             (src_facility->Clone());

  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->capacity());
  EXPECT_DOUBLE_EQ(inv_, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->RequestAmt());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, cloned_fac->input_commodities());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<agent>"
     << "<UNSPECIFIED>"
     << "<input>"
     << "  <commodities>"
     << "  <incommodity>" << commod1_ << "</incommodity>"
     << "  <incommodity>" << commod2_ << "</incommodity>"
     << "  </commodities>"
     << "  <input_capacity>" << capacity_ << "</input_capacity>"
     << "  <inventorysize>" << inv_ << "</inventorysize>"
     << "</input>"
     << "</UNSPECIFIED>"
     << "</agent>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::SinkFacility fac(tc_.get());

  //EXPECT_NO_THROW(fac.InitFrom(&engine););
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, fac.input_commodities());
  EXPECT_DOUBLE_EQ(capacity_, fac.capacity());
  EXPECT_DOUBLE_EQ(inv_, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, fac.RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, fac.InventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Requests) {
  using cyclus::Request;
  using cyclus::RequestPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;

  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> commods (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();

  ASSERT_EQ(ports.size(), 1);
  ASSERT_EQ(ports.begin()->get()->qty(), capacity_);
  const std::vector<Request<Material>*>& requests =
      ports.begin()->get()->requests();
  ASSERT_EQ(requests.size(), 2);

  for (int i = 0; i < ncommods_; ++i) {
    Request<Material>* req = *(requests.begin() + i);
    EXPECT_EQ(req->requester(), src_facility);
    EXPECT_EQ(req->commodity(), commods[i]);
  }

  const std::set< CapacityConstraint<Material> >& constraints =
      ports.begin()->get()->constraints();
  ASSERT_TRUE(constraints.size() > 0);
  EXPECT_EQ(constraints.size(), 1);
  EXPECT_EQ(*constraints.begin(), CapacityConstraint<Material>(capacity_));  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, EmptyRequests) {
  using cyclus::Material;
  using cyclus::RequestPortfolio;

  src_facility->capacity(0);
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();
  EXPECT_TRUE(ports.empty());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Accept) {
  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::Trade;
  using test_helpers::get_mat;

  double qty = qty_ * 2;
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> > responses;

  Request<Material>* req1 =
      Request<Material>::Create(get_mat(922350000, qty_), src_facility, commod1_);
  Bid<Material>* bid1 = Bid<Material>::Create(req1, get_mat(), trader);

  Request<Material>* req2 =
      Request<Material>::Create(get_mat(922350000, qty_), src_facility, commod2_);
  Bid<Material>* bid2 =
      Bid<Material>::Create(req2, get_mat(922350000, qty_), trader);

  Trade<Material> trade1(req1, bid1, qty_);
  responses.push_back(std::make_pair(trade1, get_mat(922350000, qty_)));
  Trade<Material> trade2(req2, bid2, qty_);
  responses.push_back(std::make_pair(trade2, get_mat(922350000, qty_)));

  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  src_facility->AcceptMatlTrades(responses);  
  EXPECT_DOUBLE_EQ(qty, src_facility->InventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SinkFacilityConstructor(cyclus::Context* ctx) {
  return new cycamore::SinkFacility(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityTests,
                        Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, AgentTests,
                        Values(&SinkFacilityConstructor));


