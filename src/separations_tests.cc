#include <gtest/gtest.h>

#include "separations_tests.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationsTest::SetUp() {
  src_facility_ = new Separations(tc_.get());
  InitParameters();
  SetUpSeparations();
}

void SeparationsTest::TearDown() {
  delete src_facility_;
}

void SeparationsTest::InitParameters(){
  in_c1 = "in_c1";
  out_c1 = "out_c1";
  out_c2 = "out_c2";
  out_c3 = "out_c3";
  outs.push_back(out_c1);
  outs.push_back(out_c2);
  outs.push_back(out_c3);
  process_time = 10;
  max_inv_size = 200;
  capacity = 20;
  cost = 1;

}

void SeparationsTest::SetUpSeparations(){
  src_facility_->InCommod(in_c1);
  src_facility_->OutCommods(outs);
  src_facility_->ProcessTime(process_time);
  src_facility_->MaxInvSize(max_inv_size);
  src_facility_->Capacity(capacity);
}

void SeparationsTest::TestInitState(Separations* fac){
  EXPECT_EQ(process_time, fac->ProcessTime());
  EXPECT_EQ(max_inv_size, fac->MaxInvSize());
  EXPECT_EQ(capacity, fac->Capacity());
  EXPECT_EQ(outs, fac->OutCommods());
  EXPECT_EQ(in_c1, fac->InCommod());
}

void SeparationsTest::TestRequest(Separations* fac, double cap){
  //cyclus::Material::Ptr req = fac->Request_();
  //EXPECT_EQ(cap, req->quantity());
}

void SeparationsTest::TestAddMat(Separations* fac, 
    cyclus::Material::Ptr mat){
  double amt = mat->quantity();
  double before = fac->sepbuff_quantity();
  //fac->AddMat_(mat);
  double after = fac->sepbuff_quantity();
  EXPECT_EQ(amt, after - before);
}

void SeparationsTest::TestBuffers(Separations* fac, double inv, 
    double proc, double rawbuff){
  double t = tc_.get()->time();

  EXPECT_EQ(inv, fac->sepbuff_quantity());
  EXPECT_EQ(rawbuff, fac->rawbuff.quantity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, clone) {
  Separations* cloned_fac =
      dynamic_cast<Separations*> (src_facility_->Clone());
  TestInitState(cloned_fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, CurrentCapacity) {
  EXPECT_EQ(capacity, src_facility_->current_capacity());
  src_facility_->MaxInvSize(1e299);
  EXPECT_EQ(1e299, src_facility_->MaxInvSize());
  EXPECT_EQ(capacity, src_facility_->Capacity());
  EXPECT_EQ(capacity, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test Separations specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, Request) { 
  TestRequest(src_facility_, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, AddMats) { 
  double cap = src_facility_->current_capacity();
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(0.5*cap);
  TestAddMat(src_facility_, mat);

  // cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  // cyclus::Material::Ptr recmat = cyclus::Material::CreateUntracked(0.5*cap, rec);
  //TestAddMat(src_facility_, recmat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test Separations specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, Tock) {

  // initially, nothing in the buffers
  TestBuffers(src_facility_,0,0,0);

  double cap = src_facility_->current_capacity();
  //cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  //cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  //TestAddMat(src_facility_, mat);

  // affter add, the sepbuff has the material
  TestBuffers(src_facility_,cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());

  // after tock, the processing buffer has the material
  TestBuffers(src_facility_,0,cap,0);

  EXPECT_EQ(0, tc_.get()->time());
  for( int i = 1; i < process_time-1; ++i){
    tc_.get()->time(i);
    EXPECT_NO_THROW(src_facility_->Tock());
    TestBuffers(src_facility_,0,0,0);
  }
  
  tc_.get()->time(process_time);
  EXPECT_EQ(process_time, tc_.get()->time());
  EXPECT_EQ(0, src_facility_->ready());
  src_facility_->Tock();
  TestBuffers(src_facility_,0,0,cap);

  tc_.get()->time(process_time+1);
  EXPECT_EQ(1, src_facility_->ready());
  src_facility_->Tock();
  TestBuffers(src_facility_,0,0,cap);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationsTest, NoProcessTime) {
  // tests what happens when the process time is zero
  src_facility_->ProcessTime(0);
  EXPECT_EQ(0, src_facility_->ProcessTime());

  double cap = src_facility_->current_capacity();
  //cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  //cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  //TestAddMat(src_facility_, mat);

  // affter add, the sepbuff has the material
  TestBuffers(src_facility_,cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());

  // affter tock, the rawbuff have the material
  TestBuffers(src_facility_,0,0,cap);
}


} // namespace cycamore

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationsConstructor(cyclus::Context* ctx) {
  return new cycamore::Separations(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationsFac, FacilityTests,
                        ::testing::Values(&SeparationsConstructor));

INSTANTIATE_TEST_CASE_P(SeparationsFac, AgentTests,
                        ::testing::Values(&SeparationsConstructor));
