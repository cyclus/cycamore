#include <gtest/gtest.h>

#include "facility_tests.h"
#include "agent_tests.h"
#include "resource_helpers.h"
#include "infile_tree.h"
#include "xml_parser.h"

#include "sink_tests.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::SetUp() {
  src_facility = new cycamore::Sink(tc_.get());
  trader = tc_.trader();
  InitParameters();
  SetUpSink();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::TearDown() {
  delete src_facility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::InitParameters() {
  commod1_ = "acommod";
  commod2_ = "bcommod";
  commod3_ = "ccommod";
  capacity_ = 5;
  inv_ = capacity_ * 2;
  qty_ = capacity_ * 0.5;
  ncommods_ = 2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkTest::SetUpSink() {
  src_facility->AddCommodity(commod1_);
  src_facility->AddCommodity(commod2_);
  src_facility->Capacity(capacity_);
  src_facility->SetMaxInventorySize(inv_);
  src_facility->EnterNotify();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, InitialState) {
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->Capacity());
  EXPECT_DOUBLE_EQ(inv_, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, src_facility->input_commodities());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Clone) {
  using cycamore::Sink;
  Sink* cloned_fac = dynamic_cast<cycamore::Sink*>
                             (src_facility->Clone());

  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->Capacity());
  EXPECT_DOUBLE_EQ(inv_, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->RequestAmt());
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, cloned_fac->input_commodities());

  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<config>"
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
     << "</config>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::Sink fac(tc_.get());

  // EXPECT_NO_THROW(fac.InitFrom(&engine););
  std::string arr[] = {commod1_, commod2_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, fac.input_commodities());
  EXPECT_DOUBLE_EQ(capacity_, fac.Capacity());
  EXPECT_DOUBLE_EQ(inv_, fac.MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, fac.RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, fac.InventorySize());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, EmptyRequests) {
  using cyclus::Material;
  using cyclus::RequestPortfolio;

  src_facility->Capacity(0);
  std::set<RequestPortfolio<Material>::Ptr> ports =
      src_facility->GetMatlRequests();
  EXPECT_TRUE(ports.empty());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SinkConstructor(cyclus::Context* ctx) {
  return new cycamore::Sink(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityTests, Values(&SinkConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, AgentTests, Values(&SinkConstructor));
