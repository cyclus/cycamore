#include <gtest/gtest.h>

#include "storage_tests.h"

namespace storage {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StorageTest::SetUp() {
  src_facility_ = new Storage(tc_.get());
  InitParameters();
  SetUpStorage();
}

void StorageTest::TearDown() {
  delete src_facility_;
}

void StorageTest::InitParameters(){
  in_c1 = "in_c1";
  out_c1 = "out_c1";
  in_r1 = "in_r1";
  process_time = 10;
  max_inv_size = 200;
  capacity = 20;
  cost = 1;

  cyclus::CompMap v;
  v[922350000] = 1;
  v[922380000] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_r1, recipe);
}

void StorageTest::SetUpStorage(){
  src_facility_->in_commod_(in_c1);
  src_facility_->out_commod_(out_c1);
  src_facility_->in_recipe_(in_r1);
  src_facility_->process_time_(process_time);
  src_facility_->max_inv_size_(max_inv_size);
  src_facility_->capacity_(capacity);
  src_facility_->cost_(cost);
}

void StorageTest::TestInitState(Storage* fac){
  EXPECT_EQ(process_time, fac->process_time_());
  EXPECT_EQ(max_inv_size, fac->max_inv_size_());
  EXPECT_EQ(capacity, fac->capacity_());
  EXPECT_EQ(out_c1, fac->out_commod_());
  EXPECT_EQ(in_c1, fac->in_commod_());
  EXPECT_EQ(in_r1, fac->in_recipe_());
  EXPECT_EQ(cost, fac->cost_());
}

void StorageTest::TestRequest(Storage* fac, double cap){
  cyclus::Material::Ptr req = fac->Request_();
  EXPECT_EQ(cap, req->quantity());
}

void StorageTest::TestAddMat(Storage* fac, 
    cyclus::Material::Ptr mat){
  double amt = mat->quantity();
  double before = fac->inventory.quantity();
  fac->AddMat_(mat);
  double after = fac->inventory.quantity();
  EXPECT_EQ(amt, after - before);
}

void StorageTest::TestBuffers(Storage* fac, double inv, 
    double proc, double stocks){
  double t = tc_.get()->time();

  EXPECT_EQ(inv, fac->inventory.quantity());
  EXPECT_EQ(proc, fac->processing[t].quantity());
  EXPECT_EQ(stocks, fac->stocks.quantity());
}

void StorageTest::TestStocks(Storage* fac, cyclus::CompMap v){

  cyclus::toolkit::ResourceBuff* buffer = &fac->stocks;
  Material::Ptr final_mat = cyclus::ResCast<Material>(buffer->Pop(cyclus::toolkit::ResourceBuff::BACK));
  cyclus::CompMap final_comp = final_mat->comp()->atom();
  EXPECT_EQ(final_comp,v);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, clone) {
  Storage* cloned_fac =
      dynamic_cast<Storage*> (src_facility_->Clone());
  TestInitState(cloned_fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, InitialState) {
  // Test things about the initial state of the facility here
  TestInitState(src_facility_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, CurrentCapacity) {
  EXPECT_EQ(capacity, src_facility_->current_capacity());
  src_facility_->max_inv_size_(1e299);
  EXPECT_EQ(1e299, src_facility_->max_inv_size_());
  EXPECT_EQ(capacity, src_facility_->capacity_());
  EXPECT_EQ(capacity, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility_->str());
  // Test Storage specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, Request) { 
  TestRequest(src_facility_, src_facility_->current_capacity());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, AddMats) { 
  double cap = src_facility_->current_capacity();
  cyclus::Material::Ptr mat = cyclus::NewBlankMaterial(0.5*cap);
  TestAddMat(src_facility_, mat);

  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr recmat = cyclus::Material::CreateUntracked(0.5*cap, rec);
  TestAddMat(src_facility_, recmat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, Tick) {
  ASSERT_NO_THROW(src_facility_->Tick());
  // Test Storage specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(StorageTest, Tock) {

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
TEST_F(StorageTest, NoProcessTime) {
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

TEST_F(StorageTest, NoConvert) {
// Make sure no conversion occurs

  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  TestAddMat(src_facility_, mat);

  EXPECT_NO_THROW(src_facility_->Tock());

  tc_.get()->time(process_time);
  EXPECT_EQ(0, src_facility_->ready());
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap);
  cyclus::CompMap in_rec;
  in_rec[922350000] = 1;
  in_rec[922380000] = 2;
  TestStocks(src_facility_,in_rec);
}

TEST_F(StorageTest, MultipleSmallBatches) {
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


TEST_F(StorageTest, ChangeCapacity) {
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

TEST_F(StorageTest, TwoBatchSameTime) {
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

TEST_F(StorageTest,ChangeProcessTime){
  // Initialize process time variable and add first batch
  int proc_time1 = src_facility_->process_time_();
  double cap = src_facility_->current_capacity();
  cyclus::Composition::Ptr rec = tc_.get()->GetRecipe(in_r1);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  TestAddMat(src_facility_, mat);  
  TestBuffers(src_facility_,cap,0,0);
  
  // Move material to processing
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap,0);
  
  // Add second batch
  cyclus::Material::Ptr mat1 = cyclus::Material::CreateUntracked(cap,rec);
  tc_.get()->time(8);
  TestAddMat(src_facility_,mat1);
  TestBuffers(src_facility_,cap,0,0);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap,0);
  
  // Increase process time
  src_facility_->process_time_(proc_time1+5);
  int proc_time2 = src_facility_->process_time_();
  
  // Make sure material doesn't move before new process time
  for( int i=proc_time1; i < proc_time2 - 1; ++i){
  tc_.get()->time(i);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,0);
  }
  
  // Move first batch to stocks
  tc_.get()->time(proc_time2);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap);
  
  // Decrease process time
  src_facility_->process_time_(proc_time2-3);
  int proc_time3 = src_facility_->process_time_();
  
  // Move second batch to stocks
  tc_.get()->time(proc_time3 +8);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,2*cap);
  
}

TEST_F(StorageTest,DifferentRecipe){
  // Initialize material with different recipe than in_recipe
  double cap = src_facility_->current_capacity();
  cyclus::CompMap v;
  v[922350000] = 3;
  v[922380000] = 1;
  cyclus::Composition::Ptr rec = cyclus::Composition::CreateFromAtom(v);
  cyclus::Material::Ptr mat = cyclus::Material::CreateUntracked(cap, rec);
  
  // Move material through the facility
  TestAddMat(src_facility_, mat);  
  TestBuffers(src_facility_,cap,0,0);  
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,cap,0);
  tc_.get()->time(process_time);
  EXPECT_NO_THROW(src_facility_->Tock());
  TestBuffers(src_facility_,0,0,cap);
}

} // namespace storage

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* StorageConstructor(cyclus::Context* ctx) {
  return new storage::Storage(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(StorageFac, FacilityTests,
                        ::testing::Values(&StorageConstructor));

INSTANTIATE_TEST_CASE_P(StorageFac, AgentTests,
                        ::testing::Values(&StorageConstructor));