// SourceFacilityTests.cpp
#include <gtest/gtest.h>

#include "SourceFacilityTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacilityTest::SetUp() {
  initParameters();
  setUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacilityTest::TearDown() {
  delete src_facility;
  delete commod_market;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacilityTest::initParameters() {
  commod_ = "commod";
  recipe_name_ = "recipe";
  recipe_ = CompMapPtr(new CompMap(ATOM));
  cyclus::RecipeLibrary::recordRecipe(recipe_name_,recipe_);
  commod_market = new TestMarket();
  commod_market->setcyclus::Commodity(commod_);
  cyclus::MarketModel::registerMarket(commod_market);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacilityTest::setUpSourceFacility() {
  src_facility = new SourceFacility();
  src_facility->setcyclus::Commodity(commod_);
  src_facility->setRecipe(recipe_name_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest, InitialState) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0, src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest,clone) {
  SourceFacility* cloned_fac = new SourceFacility();
  cloned_fac->cloneModuleMembersFrom(src_facility);

  EXPECT_EQ(src_facility->commodity(),cloned_fac->commodity());
  EXPECT_EQ(src_facility->capacity(),cloned_fac->capacity());
  EXPECT_EQ(src_facility->maxInventorySize(),cloned_fac->maxInventorySize());
  EXPECT_EQ(src_facility->recipe(),cloned_fac->recipe());
  
  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest, ReceiveMessage) {
  cyclus::msg_ptr msg = cyclus::msg_ptr(new cyclus::Message(src_facility));
  EXPECT_THROW(src_facility->receiveMessage(msg), cyclus::CycException);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest, Tick) {
  int time = 1;
  ASSERT_DOUBLE_EQ(0.0, src_facility->inventorySize());
  ASSERT_NO_THROW(src_facility->handleTick(time));
  EXPECT_LT(0.0, src_facility->inventorySize());
  EXPECT_LE(src_facility->capacity(), src_facility->inventorySize());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(SourceFacilityTest, Tock) {
  int time = 1;
  EXPECT_DOUBLE_EQ(0.0,src_facility->inventorySize());
  EXPECT_NO_THROW(src_facility->handleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(SourceFac, FacilityModelTests, Values(&SourceFacilityConstructor));
INSTANTIATE_TEST_CASE_P(SourceFac, ModelTests, Values(&SourceFacilityModelConstructor));
