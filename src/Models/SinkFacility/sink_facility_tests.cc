// sink_facility_tests.cc
#include <gtest/gtest.h>

#include "sink_facility_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::SetUp() {
  InitParameters();
  SetUpSinkFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::InitParameters() {
  commod1_ = "incommod";
  commod2_ = "nocommod";
  capacity_ = 5;
  inv_ = capacity_ * 2;
  qty_ = capacity_ * 0.5;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacilityTest::SetUpSinkFacility() {
  using cycamore::SinkFacility;
  src_facility = new SinkFacility(tc_.get());
  src_facility->AddCommodity(commod1_);
  src_facility->set_capacity(capacity_);
  src_facility->SetMaxInventorySize(inv_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, InitialState) {
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->capacity());
  EXPECT_DOUBLE_EQ(inv_, src_facility->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, src_facility->RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  std::string arr[] = {commod1_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, src_facility->input_commodities());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, clone) {
  using cycamore::SinkFacility;
  SinkFacility* cloned_fac = dynamic_cast<cycamore::SinkFacility*>
                             (src_facility->Clone());

  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->capacity());
  EXPECT_DOUBLE_EQ(inv_, cloned_fac->MaxInventorySize());
  EXPECT_DOUBLE_EQ(capacity_, cloned_fac->RequestAmt());
  EXPECT_DOUBLE_EQ(0.0, cloned_fac->InventorySize());
  std::string arr[] = {commod1_};
  std::vector<std::string> vexp (arr, arr + sizeof(arr) / sizeof(arr[0]) );
  EXPECT_EQ(vexp, cloned_fac->input_commodities());

  delete cloned_fac;
}

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TEST_F(SinkFacilityTest, Init) {
//   std::stringstream ss;
//   ss << "<root>" << "<output>"
//      << "<outcommodity>" << commod << "</outcommodity>"
//      << "<output_capacity>" << capacity << "</output_capacity>"
//      << "<recipe>" << recipe_name << "</recipe>"
//      << "</output>" << "</root>";

//   cyclus::XMLParser p;
//   p.Init(ss);
//   cyclus::XMLQueryEngine engine(p);
//   cycamore::SourceFacility fac(tc.get());

//   EXPECT_NO_THROW(fac.InitModuleMembers(&engine););
//   EXPECT_EQ(fac.capacity(), capacity);
//   EXPECT_EQ(fac.commodity(), commod);
//   EXPECT_EQ(fac.recipe(), recipe_name);
//   EXPECT_EQ(fac.current_capacity(), capacity);
// }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Requests) {
  std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> reqs =
      src_facility->AddMatlRequests();
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SinkFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SinkFacilityModelConstructor(cyclus::Context* ctx) {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::Model*>(new SinkFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SinkFacilityConstructor(cyclus::Context* ctx) {
  using cycamore::SinkFacility;
  return dynamic_cast<cyclus::FacilityModel*>(new SinkFacility(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SinkFac, FacilityModelTests,
                        Values(&SinkFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SinkFac, ModelTests,
                        Values(&SinkFacilityModelConstructor));


