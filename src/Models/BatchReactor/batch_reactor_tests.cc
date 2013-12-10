// batch_reactor_tests.cc

#include <sstream>

#include "commodity.h"
#include "composition.h"
#include "error.h"
#include "facility_model_tests.h"
#include "model_tests.h"
#include "model.h"
#include "xml_query_engine.h"

#include "batch_reactor_tests.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool operator==(const BatchReactor::InitCond& l,
                const BatchReactor::InitCond& r) {
  return (l.n_reserves == r.n_reserves &&
          l.n_core == r.n_core &&
          l.n_storage == r.n_storage);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::SetUp() {
  src_facility = new BatchReactor(tc_.get());
  InitParameters();
  SetUpSourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TearDown() {
  delete src_facility;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::InitParameters() {
  in_commod = "inc";
  out_commod = "outc";
  in_recipe = "inr";
  out_recipe = "outr";
  commodity = "power";
  capacity = 200;
  cost = capacity;
  
  n_batches = 5;
  n_load = 2;
  n_reserves = 3;
  process_time = 10;
  refuel_time = 2;
  preorder_time = 1;
  batch_size = 4.5;

  ic_reserves = 2;
  ic_core = 3;
  ic_storage = 1;
  
  cyclus::CompMap v;
  v[92235] = 1;
  v[92238] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_recipe, recipe);

  v[94239] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(out_recipe, recipe);

  commod1 = in_commod;
  commod2 = "inc2";
  pref1 = 5;
  pref2 = 0.5;
  commod_prefs[commod1] = pref1;
  commod_prefs[commod2] = pref2;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::SetUpSourceFacility() {
  src_facility->in_recipe(in_recipe);
  src_facility->in_commodity(in_commod);
  src_facility->out_commodity(out_commod);
  src_facility->out_recipe(out_recipe);
  src_facility->n_batches(n_batches);
  src_facility->n_load(n_load);
  src_facility->n_reserves(n_reserves);
  src_facility->process_time(process_time);
  src_facility->refuel_time(refuel_time);
  src_facility->preorder_time(preorder_time);
  src_facility->batch_size(batch_size);
  SetICs(BatchReactor::InitCond(ic_reserves, ic_core, ic_storage));
  
  src_facility->AddCommodity(commodity);
  src_facility->cyclus::CommodityProducer::SetCapacity(commodity, capacity);
  src_facility->cyclus::CommodityProducer::SetCost(commodity, capacity);

  src_facility->commod_prefs(commod_prefs);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBuffs(int nreserves, int ncore, int nstorage) {
  EXPECT_EQ(nreserves, src_facility->reserves_.count());
  EXPECT_EQ(ncore, src_facility->core_.count());
  EXPECT_EQ(nstorage, src_facility->storage_.count());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestReserveBatches(cyclus::Material::Ptr mat,
                                          int n, double qty) {
  src_facility->AddBatches_(mat);
  EXPECT_EQ(n, src_facility->reserves_.count());
  EXPECT_DOUBLE_EQ(qty, src_facility->spillover_->quantity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBatchIn(int n_core, int n_reserves) {
  src_facility->MoveBatchIn_();
  EXPECT_EQ(n_core, src_facility->n_core());
  EXPECT_EQ(n_reserves, src_facility->reserves_.count());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBatchOut(int n_core, int n_storage) {
  src_facility->MoveBatchOut_();
  EXPECT_EQ(n_core, src_facility->n_core());
  EXPECT_EQ(n_storage, src_facility->storage_.count());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, InitialState) {
  EXPECT_EQ(in_recipe, src_facility->in_recipe());
  EXPECT_EQ(in_commod, src_facility->in_commodity());
  EXPECT_EQ(out_commod, src_facility->out_commodity());
  EXPECT_EQ(out_recipe, src_facility->out_recipe());
  EXPECT_EQ(n_batches, src_facility->n_batches());
  EXPECT_EQ(n_load, src_facility->n_load());
  EXPECT_EQ(n_reserves, src_facility->n_reserves());
  EXPECT_EQ(process_time, src_facility->process_time());
  EXPECT_EQ(refuel_time, src_facility->refuel_time());
  EXPECT_EQ(preorder_time, src_facility->preorder_time());
  EXPECT_EQ(batch_size, src_facility->batch_size());
  EXPECT_EQ(0, src_facility->n_core());
  EXPECT_EQ(BatchReactor::INITIAL, src_facility->phase());
  EXPECT_EQ(BatchReactor::InitCond(ic_reserves, ic_core, ic_storage),
            src_facility->ics());

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(src_facility->ProducesCommodity(commod));
  EXPECT_EQ(capacity, src_facility->ProductionCapacity(commod));
  EXPECT_EQ(cost, src_facility->ProductionCost(commod));

  EXPECT_EQ(commod_prefs, src_facility->commod_prefs());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "  <fuel_input>"
     << "    <incommodity>" << in_commod << "</incommodity>"
     << "    <inrecipe>" << in_recipe << "</inrecipe>"
     << "  </fuel_input>"
     << "  <fuel_output>"
     << "    <outcommodity>" << out_commod << "</outcommodity>"
     << "    <outrecipe>" << out_recipe << "</outrecipe>"
     << "  </fuel_output>"
     << "  <processtime>" << process_time << "</processtime>"
     << "  <nbatches>" << n_batches << "</nbatches>"
     << "  <batchsize>" << batch_size << "</batchsize>"
     << "  <refueltime>" << refuel_time << "</refueltime>"
     << "  <orderlookahead>" << preorder_time << "</orderlookahead>"
     << "  <norder>" << n_reserves << "</norder>"
     << "  <nreload>" << n_load << "</nreload>"
     << "  <initial_condition>"
     << "    <nreserves>" << ic_reserves << "</nreserves>"
     << "    <ncore>" << ic_core << "</ncore>"
     << "    <nstorage>" << ic_storage << "</nstorage>"
     << "  </initial_condition>"
     << "  <commodity_production>"
     << "    <commodity>" << commodity << "</commodity>"
     << "    <capacity>" << capacity << "</capacity>"
     << "    <cost>" << cost << "</cost>"
     << "  </commodity_production>"
     << "  <commod_pref>"
     << "    <incommodity>" << commod1 << "</incommodity>"
     << "    <preference>" << pref1 << "</preference>"
     << "  </commod_pref>"
     << "  <commod_pref>"
     << "    <incommodity>" << commod2 << "</incommodity>"
     << "    <preference>" << pref2 << "</preference>"
     << "  </commod_pref>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::XMLQueryEngine engine(p);
  cycamore::BatchReactor fac(tc_.get());
  
  EXPECT_NO_THROW(fac.InitModuleMembers(&engine););
  EXPECT_EQ(in_recipe, fac.in_recipe());
  EXPECT_EQ(in_commod, fac.in_commodity());
  EXPECT_EQ(out_commod, fac.out_commodity());
  EXPECT_EQ(out_recipe, fac.out_recipe());
  EXPECT_EQ(n_batches, fac.n_batches());
  EXPECT_EQ(n_load, fac.n_load());
  EXPECT_EQ(n_reserves, fac.n_reserves());
  EXPECT_EQ(process_time, fac.process_time());
  EXPECT_EQ(refuel_time, fac.refuel_time());
  EXPECT_EQ(preorder_time, fac.preorder_time());
  EXPECT_EQ(batch_size, fac.batch_size());
  EXPECT_EQ(0, fac.n_core());
  EXPECT_EQ(BatchReactor::INITIAL, fac.phase());
  EXPECT_EQ(BatchReactor::InitCond(ic_reserves, ic_core, ic_storage), fac.ics());
  
  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(fac.ProducesCommodity(commod));
  EXPECT_EQ(capacity, fac.ProductionCapacity(commod));
  EXPECT_EQ(cost, fac.ProductionCost(commod));

  EXPECT_EQ(commod_prefs, fac.commod_prefs());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Clone) {
  cycamore::BatchReactor* cloned_fac =
    dynamic_cast<cycamore::BatchReactor*>(src_facility->Clone());

  EXPECT_EQ(in_recipe, cloned_fac->in_recipe());
  EXPECT_EQ(in_commod, cloned_fac->in_commodity());
  EXPECT_EQ(out_commod, cloned_fac->out_commodity());
  EXPECT_EQ(out_recipe, cloned_fac->out_recipe());
  EXPECT_EQ(n_batches, cloned_fac->n_batches());
  EXPECT_EQ(n_load, cloned_fac->n_load());
  EXPECT_EQ(n_reserves, cloned_fac->n_reserves());
  EXPECT_EQ(process_time, cloned_fac->process_time());
  EXPECT_EQ(refuel_time, cloned_fac->refuel_time());
  EXPECT_EQ(preorder_time, cloned_fac->preorder_time());
  EXPECT_EQ(batch_size, cloned_fac->batch_size());
  EXPECT_EQ(0, cloned_fac->n_core());
  EXPECT_EQ(BatchReactor::INITIAL, cloned_fac->phase());
  EXPECT_EQ(BatchReactor::InitCond(ic_reserves, ic_core, ic_storage),
            cloned_fac->ics());

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(cloned_fac->ProducesCommodity(commod));
  EXPECT_EQ(capacity, cloned_fac->ProductionCapacity(commod));
  EXPECT_EQ(cost, cloned_fac->ProductionCost(commod));
  
  EXPECT_EQ(commod_prefs, cloned_fac->commod_prefs());
  
  delete cloned_fac;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Tick) {
  int time = 1;
  EXPECT_NO_THROW(src_facility->HandleTick(time););
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_facility->HandleTock(time));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, StartProcess) {
  int t = tc_.get()->time();
  src_facility->phase(BatchReactor::PROCESS);
  EXPECT_EQ(t, src_facility->start_time());
  EXPECT_EQ(t + process_time, src_facility->end_time());
  EXPECT_EQ(t + process_time - preorder_time, src_facility->order_time());
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, InitCond) {
  src_facility->Deploy(src_facility);
  TestBuffs(ic_reserves, ic_core, ic_storage);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, AddBatches) {
  using cyclus::Material;
  
  Material::Ptr mat = Material::CreateBlank(batch_size);
  // mat to add, nreserves, qty of spillover
  TestReserveBatches(mat, 1, 0);

  mat = Material::CreateBlank(batch_size - (1 + cyclus::eps()));
  TestReserveBatches(mat, 1, batch_size - (1 + cyclus::eps()));
  
  mat = Material::CreateBlank((1 + cyclus::eps()));
  TestReserveBatches(mat, 2, 0);

  mat = Material::CreateBlank(batch_size + (1 + cyclus::eps()));
  TestReserveBatches(mat, 3, 1 + cyclus::eps());
  
  mat = Material::CreateBlank(batch_size - (1 + cyclus::eps()));
  TestReserveBatches(mat, 4, 0);
  
  mat = Material::CreateBlank(1 + cyclus::eps());
  TestReserveBatches(mat, 4, 1 + cyclus::eps());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, BatchInOut) {
  using cyclus::Material;

  EXPECT_THROW(TestBatchIn(1, 0), cyclus::Error);
  
  Material::Ptr mat = Material::CreateBlank(batch_size);
  TestReserveBatches(mat, 1, 0);
  TestBatchIn(1, 0);

  mat = Material::CreateBlank(batch_size * 2);
  TestReserveBatches(mat, 2, 0);
  TestBatchIn(2, 1);
  
  TestBatchOut(1, 1);
  TestBatchOut(0, 2);

  EXPECT_THROW(TestBatchOut(1, 0), cyclus::Error);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* BatchReactorModelConstructor(cyclus::Context* ctx) {
  using cycamore::BatchReactor;
  return dynamic_cast<cyclus::Model*>(new BatchReactor(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* BatchReactorConstructor(cyclus::Context* ctx) {
  using cycamore::BatchReactor;
  return dynamic_cast<cyclus::FacilityModel*>(new BatchReactor(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(BatchReactor, FacilityModelTests,
                        Values(&BatchReactorConstructor));
INSTANTIATE_TEST_CASE_P(BatchReactor, ModelTests,
                        Values(&BatchReactorModelConstructor));

} // namespace cycamore
