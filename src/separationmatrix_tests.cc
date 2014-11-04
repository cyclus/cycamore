#include <gtest/gtest.h>

#include "separationmatrix_tests.h"

namespace separationmatrix {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrixTest::SetUp() {
  src_facility_ = new SeparationMatrix(tc_.get());
  InitParameters();
  SetUpSeparationMatrix();
}

void SeparationMatrixTest::TearDown() {
  delete src_facility_;
}

void SeparationMatrixTest::InitParameters(){
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

void SeparationMatrixTest::SetUpSeparationMatrix(){
  src_facility_->in_commod_(in_c1);
  src_facility_->out_commods_(outs);
  src_facility_->process_time_(process_time);
  src_facility_->max_inv_size_(max_inv_size);
  src_facility_->capacity_(capacity);
}

void SeparationMatrixTest::TestInitState(SeparationMatrix* fac){
  EXPECT_EQ(process_time, fac->process_time_());
  EXPECT_EQ(max_inv_size, fac->max_inv_size_());
  EXPECT_EQ(capacity, fac->capacity_());
  EXPECT_EQ(outs, fac->out_commods_());
  EXPECT_EQ(in_c1, fac->in_commod_());
}

void SeparationMatrixTest::TestRequest(SeparationMatrix* fac, double cap){
  //cyclus::Material::Ptr req = fac->Request_();
  //EXPECT_EQ(cap, req->quantity());
}

void SeparationMatrixTest::TestAddMat(SeparationMatrix* fac, 
    cyclus::Material::Ptr mat){
  double amt = mat->quantity();
  double before = fac->sepbuff_quantity();
  //fac->AddMat_(mat);
  double after = fac->sepbuff_quantity();
  EXPECT_EQ(amt, after - before);
}

void SeparationMatrixTest::TestBuffers(SeparationMatrix* fac, double inv, 
    double proc, double rawbuff){
  double t = tc_.get()->time();

  EXPECT_EQ(inv, fac->sepbuff_quantity());
  EXPECT_EQ(rawbuff, fac->rawbuff.quantity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, clone) {
  SeparationMatrix* cloned_fac =
      dynamic_cast<SeparationMatrix*> (src_facility_->Clone());
  TestInitState(cloned_fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, CurrentCapacity) {
  EXPECT_EQ(capacity, src_facility_->current_capacity());
  src_facility_->max_inv_size_(1e299);
  EXPECT_EQ(1e299, src_facility_->max_inv_size_());
  EXPECT_EQ(capacity, src_facility_->capacity_());
  EXPECT_EQ(capacity, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test SeparationMatrix specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Request) { 
  TestRequest(src_facility_, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, AddMats) { 
  double cap = src_facility_->current_capacity();
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(0.5*cap);
  TestAddMat(src_facility_, mat);

  // cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  // cyclus::Material::Ptr recmat = cyclus::Material::CreateUntracked(0.5*cap, rec);
  //TestAddMat(src_facility_, recmat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test SeparationMatrix specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SeparationMatrixTest, Tock) {

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
TEST_F(SeparationMatrixTest, NoProcessTime) {
  // tests what happens when the process time is zero
  src_facility_->process_time_(0);
  EXPECT_EQ(0, src_facility_->process_time_());

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


} // namespace separationmatrix

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* SeparationMatrixConstructor(cyclus::Context* ctx) {
  return new separationmatrix::SeparationMatrix(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SeparationMatrixFac, FacilityTests,
                        ::testing::Values(&SeparationMatrixConstructor));

INSTANTIATE_TEST_CASE_P(SeparationMatrixFac, AgentTests,
                        ::testing::Values(&SeparationMatrixConstructor));
