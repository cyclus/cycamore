// source_facility_tests.cc
#include <gtest/gtest.h>

#include "source_facility_tests.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::SetUp() {
  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::TearDown() {
  delete src_facility;
  delete commod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::InitParameters() {
  commod_ = "commod";
  recipe_name_ = "recipe";
  cyclus::Context* ctx = tc_.get();

  recipe_ = cyclus::Composition::CreateFromAtom(cyclus::CompMap());
  ctx->AddRecipe(recipe_name_, recipe_);
  commod_market = new TestMarket(ctx);
  commod_market->SetCommodity(commod_);
  cyclus::MarketModel::RegisterMarket(commod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacilityTest::SetUpSourceFacility() {
  cyclus::Context* ctx = tc_.get();
  src_facility = new cycamore::SourceFacility(ctx);
  src_facility->SetCommodity(commod_);
  src_facility->SetRecipe(recipe_name_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, clone) {
  cyclus::Context* ctx = tc_.get();
  cycamore::SourceFacility* cloned_fac = dynamic_cast<cycamore::SourceFacility*>
                                         (src_facility->Clone());

  EXPECT_EQ(src_facility->commodity(), cloned_fac->commodity());
  EXPECT_EQ(src_facility->capacity(), cloned_fac->capacity());
  EXPECT_EQ(src_facility->MaxInventorySize(), cloned_fac->MaxInventorySize());
  EXPECT_EQ(src_facility->recipe(), cloned_fac->recipe());

  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, ReceiveMessage) {
  cyclus::Message::Ptr msg = cyclus::Message::Ptr(new cyclus::Message(
                                                    src_facility));
  EXPECT_THROW(src_facility->ReceiveMessage(msg), cyclus::Error);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Tick) {
  int time = 1;
  ASSERT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  ASSERT_NO_THROW(src_facility->HandleTick(time));
  EXPECT_LT(0.0, src_facility->InventorySize());
  EXPECT_LE(src_facility->capacity(), src_facility->InventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SourceFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->InventorySize());
  EXPECT_NO_THROW(src_facility->HandleTock(time));
}

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
