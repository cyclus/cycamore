#include <gtest/gtest.h>

#include "commodconverter_tests.h"

namespace commodconverter {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodConverterTest::SetUp() {
  src_facility_ = new CommodConverter(tc_.get());
  InitParameters();
  SetUpCommodConverter();
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
  cost = 1;

  cyclus::CompMap v;
  v[922350000] = 1;
  v[922380000] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_r1, recipe);

  v[94239] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(out_r1, recipe);
}

void CommodConverterTest::SetUpCommodConverter(){
  src_facility_->in_commod_(in_c1);
  src_facility_->out_commod_(out_c1);
  src_facility_->in_recipe_(in_r1);
  src_facility_->out_recipe_(out_r1);
  src_facility_->process_time_(process_time);
  src_facility_->max_inv_size_(max_inv_size);
  src_facility_->capacity_(capacity);
  src_facility_->cost_(cost);
}

void CommodConverterTest::TestInitState(CommodConverter* fac){
  EXPECT_EQ(process_time, fac->process_time_());
  EXPECT_EQ(max_inv_size, fac->max_inv_size_());
  EXPECT_EQ(capacity, fac->capacity_());
  EXPECT_EQ(out_c1, fac->out_commod_());
  EXPECT_EQ(out_r1, fac->out_recipe_());
  EXPECT_EQ(in_c1, fac->in_commod_());
  EXPECT_EQ(in_r1, fac->in_recipe_());
  EXPECT_EQ(cost, fac->cost_());
}

void CommodConverterTest::TestRequest(CommodConverter* fac, double cap){
  cyclus::Material::Ptr req = fac->Request_();
  EXPECT_EQ(cap, req->quantity());
}

void CommodConverterTest::TestAddMat(CommodConverter* fac, 
    cyclus::Material::Ptr mat){
  double amt = mat->quantity();
  double before = fac->inventory.quantity();
  fac->AddMat_(mat);
  double after = fac->inventory.quantity();
  EXPECT_EQ(amt, after - before);
}

void CommodConverterTest::TestBuffers(CommodConverter* fac, double inv, 
    double proc, double stocks){
  double t = tc_.get()->time();

  EXPECT_EQ(inv, fac->inventory.quantity());
  EXPECT_EQ(proc, fac->processing[t].quantity());
  EXPECT_EQ(stocks, fac->stocks.quantity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, clone) {
  CommodConverter* cloned_fac =
      dynamic_cast<CommodConverter*> (src_facility_->Clone());
  TestInitState(cloned_fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, CurrentCapacity) {
  EXPECT_EQ(capacity, src_facility_->current_capacity());
  src_facility_->max_inv_size_(1e299);
  EXPECT_EQ(1e299, src_facility_->max_inv_size_());
  EXPECT_EQ(capacity, src_facility_->capacity_());
  EXPECT_EQ(capacity, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test CommodConverter specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Request) { 
  TestRequest(src_facility_, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, AddMats) { 
  double cap = src_facility_->current_capacity();
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(0.5*cap);
  TestAddMat(src_facility_, mat);

  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr recmat = cyclus::Material::CreateUntracked(0.5*cap, rec);
  TestAddMat(src_facility_, recmat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test CommodConverter specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(CommodConverterTest, Tock) {

  // initially, nothing in the buffers
  TestBuffers(src_facility_,0,0,0);

  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  TestAddMat(src_facility_, mat);

  // affter add, the inventory has the material
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
TEST_F(CommodConverterTest, NoProcessTime) {
  // tests what happens when the process time is zero
  src_facility_->process_time_(0);
  EXPECT_EQ(0, src_facility_->process_time_());

  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  TestAddMat(src_facility_, mat);

  // affter add, the inventory has the material
  TestBuffers(src_facility_,cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());

  // affter tock, the stocks have the material
  TestBuffers(src_facility_,0,0,cap);
}

TEST_F(CommodConverterTest, NoConvert) {
  // test what happens if no recipe change specified
  src_facility_->out_recipe_(in_r1);
  EXPECT_EQ(src_facility_->in_recipe_(), src_facility_->out_recipe_());

  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  TestAddMat(src_facility_, mat);

  EXPECT_NO_THROW(src_facility_->Tock());

  tc_.get()->time(process_time);
  EXPECT_EQ(0, src_facility_->ready());
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap);
}

TEST_F(CommodConverterTest, MultipleSmallBatches) {
  // Add first small batch
  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(0.2*cap, rec);
  TestAddMat(src_facility_, mat);

  // After add, material is in inventory
  TestBuffers(src_facility_,0.2*cap,0,0);
  
  // Move first batch into processing
  src_facility_->Tock();
  TestBuffers(src_facility_,0,0.2*cap,0);

  // Add second small batch
  tc_.get()->time(2);
  src_facility_->Tock();
  cyclus::Material::Ptr mat1 = cyclus::Material::CreateUntracked(0.3*cap, rec);
  TestAddMat(src_facility_,mat1);
  TestBuffers(src_facility_,0.3*cap,0,0);

  // Move second batch into processing
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0.3*cap,0);

  // Move first batch to stocks
  tc_.get()->time(process_time);
  EXPECT_EQ(0,src_facility_->ready());
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,0.2*cap);

  tc_.get()->time(process_time+2);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,0.5*cap);
}


TEST_F(CommodConverterTest, ChangeCapacity) {
  src_facility_->max_inv_size_(10000);
  // Set capacity, add first batch
  src_facility_->capacity_(300);
  double cap1 = src_facility_->capacity_();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat1 = cyclus::Material::CreateUntracked(cap1, rec);
  TestAddMat(src_facility_, mat1);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap1,0);
	
  // Increase capacity, add second and third batches
  tc_.get()->time(2);
  src_facility_->capacity_(500);
  double cap2 = src_facility_->capacity_();
  cyclus::Material::Ptr mat2 = cyclus::Material::CreateUntracked(cap2,rec);
  TestAddMat(src_facility_, mat2);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap2,0);
  tc_.get()->time(3);
  cyclus::Material::Ptr mat3 = cyclus::Material::CreateUntracked(cap2,rec);
  TestAddMat(src_facility_, mat3);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap2,0);
	
  // Move first batch to stocks
  tc_.get()->time(process_time);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap1);	
	
  // Decrease capacity and move portion of second batch to stocks
  src_facility_->capacity_(400);
  tc_.get()->time(process_time+2);
  EXPECT_NO_THROW(src_facility_->Tock());   
  TestBuffers(src_facility_,0,0,cap1+400);

  // Continue to move second batch // and portion of third
  tc_.get()->time(process_time+3);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap1+cap2+300);
  
  // Move remainder of third batch
  tc_.get()->time(process_time+4);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap1+cap2+cap2);
	
}

TEST_F(CommodConverterTest, TwoBatchSameTime) {
  // Add first small batch
  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(0.2*cap, rec);
  TestAddMat(src_facility_, mat);

  // After add, material is in inventory
  TestBuffers(src_facility_,0.2*cap,0,0);
  cyclus::Material::Ptr mat1 = cyclus::Material::CreateUntracked(0.2*cap, rec);
  TestAddMat(src_facility_, mat1);
  TestBuffers(src_facility_,0.4*cap,0,0);

  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0.4*cap,0);

  // Move material to stocks
  tc_.get()->time(process_time);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,0.4*cap);
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