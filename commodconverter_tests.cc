#include <gtest/gtest.h>

#include "commodconverter_tests.h"

namespace commodconverter {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodConverterTest::SetUp() {
  src_facility_ = new CommodConverter(tc_.get());
  InitParameters();
}

void CommodConverterTest::TearDown() {
  delete src_facility_;
}

void CommodConverterTest::InitParameters(){
  in_c1 = "in_c1";
  out_c1 = "out_c1";
  in_r1 = "in_r1";
  out_r1 = "out_r1";
  process_time = 10;
  max_inv_size = 200;
  capacity = 20;
  //crctx.AddInCommod(in_c1, in_r1, out_c1, out_r1);
  //crctx.AddInCommod(in_c2, in_r2, out_c2, out_r2);


  // init conds
  //rsrv_c = in_c1;
  //rsrv_r = in_r1;
  //stor_c = out_c1;
  //stor_r = out_r1;
  //rsrv_n = 2;
  //core_n = 3;
  //stor_n = 1;
  //ics.AddReserves(rsrv_n, rsrv_r, rsrv_c);
  //ics.AddCore(core_n, core_r, core_c);
  //ics.AddStorage(stor_n, stor_r, stor_c);

  cyclus::CompMap v;
  v[922350000] = 1;
  v[922380000] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_r1, recipe);

  v[94239] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(out_r1, recipe);
}

void CommodConverterTest::TestInitState(CommodConverter* fac){
  EXPECT_EQ(process_time, fac->process_time_());
  EXPECT_EQ(max_inv_size, fac->max_inv_size_());
  EXPECT_EQ(capacity, fac->capacity_());
  EXPECT_EQ(out_c1, fac->out_commod_());
  EXPECT_EQ(out_r1, fac->out_recipe_());
  EXPECT_EQ(in_c1, fac->in_commod_());
  EXPECT_EQ(in_r1, fac->in_recipe_());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, clone) {
  CommodConverter* cloned_fac =
      dynamic_cast<CommodConverter*> (src_facility_->Clone());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test CommodConverter specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test CommodConverter specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tock) {
  EXPECT_NO_THROW(src_facility_->Tock());
  // Test CommodConverter specific behaviors of the Tock function here
}

} // namespace commodconverter

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* CommodConverterConstructor(cyclus::Context* ctx) {
  return new commodconverter::CommodConverter(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(CommodConverterFac, FacilityTests,
                        ::testing::Values(&CommodConverterConstructor));

INSTANTIATE_TEST_CASE_P(CommodConverterFac, AgentTests,
                        ::testing::Values(&CommodConverterConstructor));
