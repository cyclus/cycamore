// batch_reactor_tests.cc
#include "batch_reactor_tests.h"

#include <sstream>

#include "composition.h"
#include "error.h"
#include "facility_tests.h"
#include "agent_tests.h"
#include "agent.h"
#include "infile_tree.h"

#include "toolkit/commodity.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool operator==(const BatchReactor::InitCond& l,
                const BatchReactor::InitCond& r) {
  bool reserves = (l.n_reserves != 0 &&
                   l.n_reserves == r.n_reserves &&
                   l.reserves_rec == r.reserves_rec &&
                   l.reserves_commod == r.reserves_commod);
  bool core = (l.n_core != 0 &&
               l.n_core == r.n_core &&
               l.core_rec == r.core_rec &&
               l.core_commod == r.core_commod);
  bool storage = (l.n_storage != 0 &&
                  l.n_storage == r.n_storage &&
                  l.storage_rec == r.storage_rec &&
                  l.storage_commod == r.storage_commod);
  return (reserves && core && storage);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::SetUp() {
  src_facility = new BatchReactor(tc_.get());
  InitParameters();
  SetUpSource();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TearDown() {
  delete src_facility;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::InitParameters() {
  // init params
  in_c1 = "in_c1";
  in_c2 = "in_c2";
  out_c1 = "out_c1";
  out_c2 = "out_c2";
  in_r1 = "in_r1";
  in_r2 = "in_r2";
  out_r1 = "out_r1";
  out_r2 = "out_r2";
  crctx.AddInCommod(in_c1, in_r1, out_c1, out_r1);
  crctx.AddInCommod(in_c2, in_r2, out_c2, out_r2);

  n_batches = 5;
  n_load = 2;
  n_reserves = 3;
  process_time = 10;
  refuel_time = 2;
  preorder_time = 1;
  batch_size = 4.5;

  commodity = "power";
  capacity = 200;
  cost = capacity;

  // init conds
  rsrv_c = in_c1;
  rsrv_r = in_r1;
  core_c = in_c2;
  core_r = in_r2;
  stor_c = out_c1;
  stor_r = out_r1;
  rsrv_n = 2;
  core_n = 3;
  stor_n = 1;
  ics.AddReserves(rsrv_n, rsrv_r, rsrv_c);
  ics.AddCore(core_n, core_r, core_c);
  ics.AddStorage(stor_n, stor_r, stor_c);

  // commod prefs
  frompref1 = 7.5;
  topref1 = frompref1 - 1;
  frompref2 = 5.5;
  topref2 = frompref2 - 2;
  commod_prefs[in_c1] = frompref1;
  commod_prefs[in_c2] = frompref2;

  // changes
  change_time = 5;
  pref_changes[change_time].push_back(std::make_pair(in_c1, topref1));
  pref_changes[change_time].push_back(std::make_pair(in_c2, topref2));
  recipe_changes[change_time].push_back(std::make_pair(in_c1, in_r2));

  cyclus::CompMap v;
  v[922350000] = 1;
  v[922380000] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_r1, recipe);
  tc_.get()->AddRecipe(in_r2, recipe);

  v[94239] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(out_r1, recipe);
  tc_.get()->AddRecipe(out_r2, recipe);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::SetUpSource() {
  src_facility->crctx(crctx);
  src_facility->n_batches(n_batches);
  src_facility->n_load(n_load);
  src_facility->n_reserves(n_reserves);
  src_facility->process_time(process_time);
  src_facility->refuel_time(refuel_time);
  src_facility->preorder_time(preorder_time);
  src_facility->batch_size(batch_size);
  src_facility->ics(ics);

  src_facility->Add(commodity);
  src_facility->cyclus::toolkit::CommodityProducer::SetCapacity(commodity, capacity);
  src_facility->cyclus::toolkit::CommodityProducer::SetCost(commodity, capacity);

  src_facility->commod_prefs(commod_prefs);

  src_facility->pref_changes_[change_time].push_back(
      std::make_pair(in_c1, topref1));
  src_facility->pref_changes_[change_time].push_back(
      std::make_pair(in_c2, topref2));
  src_facility->recipe_changes_[change_time].push_back(
      std::make_pair(in_c1, in_r2));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBuffs(int nreserves, int ncore, int nstorage) {
  EXPECT_EQ(nreserves, src_facility->reserves_.count());
  EXPECT_EQ(ncore, src_facility->core_.count());
  EXPECT_EQ(nstorage, src_facility->storage_[out_c1].count());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestReserveBatches(cyclus::Material::Ptr mat,
                                          std::string commod,
                                          int n,
                                          double qty) {
  src_facility->AddBatches_(commod, mat);
  EXPECT_EQ(n, src_facility->reserves_.count());
  EXPECT_DOUBLE_EQ(qty, src_facility->spillover_->quantity());

  cyclus::Material::Ptr back = cyclus::toolkit::ResCast<cyclus::Material>(
      src_facility->reserves_.Pop(cyclus::toolkit::ResourceBuff::BACK));
  EXPECT_EQ(commod, src_facility->crctx_.commod(back));
  src_facility->reserves_.Push(back);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBatchIn(int n_core, int n_reserves) {
  src_facility->MoveBatchIn_();
  EXPECT_EQ(n_core, src_facility->n_core());
  EXPECT_EQ(n_reserves, src_facility->reserves_.count());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestBatchOut(int n_core, int n_storage) {
  src_facility->MoveBatchOut_();
  EXPECT_EQ(n_core, src_facility->n_core());
  EXPECT_EQ(n_storage, src_facility->storage_[out_c1].count());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::TestInitState(BatchReactor* fac) {
  EXPECT_EQ(crctx, fac->crctx());
  EXPECT_EQ(n_batches, fac->n_batches());
  EXPECT_EQ(n_load, fac->n_load());
  EXPECT_EQ(n_reserves, fac->n_reserves());
  EXPECT_EQ(process_time, fac->process_time());
  EXPECT_EQ(refuel_time, fac->refuel_time());
  EXPECT_EQ(preorder_time, fac->preorder_time());
  EXPECT_EQ(batch_size, fac->batch_size());
  EXPECT_EQ(0, fac->n_core());
  EXPECT_EQ(BatchReactor::INITIAL, fac->phase());
  EXPECT_EQ(ics, fac->ics());

  cyclus::toolkit::Commodity commod(commodity);
  EXPECT_TRUE(fac->Produces(commod));
  EXPECT_EQ(capacity, fac->Capacity(commod));
  EXPECT_EQ(cost, fac->Cost(commod));

  EXPECT_EQ(commod_prefs, fac->commod_prefs());

  EXPECT_EQ(pref_changes, fac->pref_changes_);
  EXPECT_EQ(recipe_changes, fac->recipe_changes_);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, InitialState) {
  TestInitState(src_facility);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, DISABLED_XMLInit) {
  std::stringstream ss;
  ss << "<start>"
     << "<name>fooname</name>"
     << "<agent>"
     << "<UNSPECIFIED>"
     << "  <fuel>"
     << "    <incommodity>" << in_c1 << "</incommodity>"
     << "    <inrecipe>" << in_r1 << "</inrecipe>"
     << "    <outcommodity>" << out_c1 << "</outcommodity>"
     << "    <outrecipe>" << out_r1 << "</outrecipe>"
     << "  </fuel>"
     << "  <fuel>"
     << "    <incommodity>" << in_c2 << "</incommodity>"
     << "    <inrecipe>" << in_r2 << "</inrecipe>"
     << "    <outcommodity>" << out_c2 << "</outcommodity>"
     << "    <outrecipe>" << out_r2 << "</outrecipe>"
     << "  </fuel>"
     << "  <processtime>" << process_time << "</processtime>"
     << "  <nbatches>" << n_batches << "</nbatches>"
     << "  <batchsize>" << batch_size << "</batchsize>"
     << "  <refueltime>" << refuel_time << "</refueltime>"
     << "  <orderlookahead>" << preorder_time << "</orderlookahead>"
     << "  <norder>" << n_reserves << "</norder>"
     << "  <nreload>" << n_load << "</nreload>"
     << "  <initial_condition>"
     << "    <reserves>"
     << "      <nbatches>" << rsrv_n << "</nbatches>"
     << "      <commodity>" << rsrv_c << "</commodity>"
     << "      <recipe>" << rsrv_r << "</recipe>"
     << "    </reserves>"
     << "    <core>"
     << "      <nbatches>" << core_n << "</nbatches>"
     << "      <commodity>" << core_c << "</commodity>"
     << "      <recipe>" << core_r << "</recipe>"
     << "    </core>"
     << "    <storage>"
     << "      <nbatches>" << stor_n << "</nbatches>"
     << "      <commodity>" << stor_c << "</commodity>"
     << "      <recipe>" << stor_r << "</recipe>"
     << "    </storage>"
     << "  </initial_condition>"
     << "  <recipe_change>"
     << "    <incommodity>" << in_c1 << "</incommodity>"
     << "    <new_recipe>" << in_r2 << "</new_recipe>"
     << "    <time>" << change_time << "</time>"
     << "  </recipe_change>"
     << "  <commodity_production>"
     << "    <commodity>" << commodity << "</commodity>"
     << "    <capacity>" << capacity << "</capacity>"
     << "    <cost>" << cost << "</cost>"
     << "  </commodity_production>"
     << "  <commod_pref>"
     << "    <incommodity>" << in_c1 << "</incommodity>"
     << "    <preference>" << frompref1 << "</preference>"
     << "  </commod_pref>"
     << "  <commod_pref>"
     << "    <incommodity>" << in_c2 << "</incommodity>"
     << "    <preference>" << frompref2 << "</preference>"
     << "  </commod_pref>"
     << "  <pref_change>"
     << "    <incommodity>" << in_c1 << "</incommodity>"
     << "    <new_pref>" << topref1 << "</new_pref>"
     << "    <time>" << change_time << "</time>"
     << "  </pref_change>"
     << "  <pref_change>"
     << "    <incommodity>" << in_c2 << "</incommodity>"
     << "    <new_pref>" << topref2 << "</new_pref>"
     << "    <time>" << change_time << "</time>"
     << "  </pref_change>"
     << "</UNSPECIFIED>"
     << "</agent>"
     << "</start>";

  cyclus::XMLParser p;
  p.Init(ss);
  cyclus::InfileTree engine(p);
  cycamore::BatchReactor* fac = new cycamore::BatchReactor(tc_.get());
  //fac->InitFrom(&engine);

  TestInitState(fac);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Clone) {
  cycamore::BatchReactor* cloned_fac =
      dynamic_cast<cycamore::BatchReactor*>(src_facility->Clone());
  TestInitState(cloned_fac);
  delete cloned_fac;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Tick) {
  EXPECT_EQ(src_facility->commod_prefs().at(in_c1), frompref1);
  EXPECT_EQ(src_facility->commod_prefs().at(in_c2), frompref2);
  EXPECT_EQ(src_facility->crctx().in_recipe(in_c1), in_r1);
  EXPECT_NO_THROW(src_facility->Tick(change_time););
  EXPECT_EQ(src_facility->commod_prefs().at(in_c1), topref1);
  EXPECT_EQ(src_facility->commod_prefs().at(in_c2), topref2);
  EXPECT_EQ(src_facility->crctx().in_recipe(in_c1), in_r2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, Tock) {
  int time = 1;
  EXPECT_NO_THROW(src_facility->Tock(time));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, StartProcess) {
  int t = tc_.get()->time();
  src_facility->phase(BatchReactor::PROCESS);
  EXPECT_EQ(t, src_facility->start_time());
  EXPECT_EQ(t + process_time - 1, src_facility->end_time());
  EXPECT_EQ(t + process_time - preorder_time - 1, src_facility->order_time());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, InitCond) {
  cyclus::Env::SetNucDataPath();
  src_facility->Build(NULL);
  TestBuffs(rsrv_n, core_n, stor_n);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, AddBatches) {
  using cyclus::Material;

  Material::Ptr mat = cyclus::NewBlankMaterial(batch_size);
  // mat to add, commodity, reserves, qty of spillover
  TestReserveBatches(mat, in_c1, 1, 0);

  mat = cyclus::NewBlankMaterial(batch_size - (1 + cyclus::eps()));
  TestReserveBatches(mat, in_c1, 1, batch_size - (1 + cyclus::eps()));

  mat = cyclus::NewBlankMaterial((1 + cyclus::eps()));
  TestReserveBatches(mat, in_c1, 2, 0);

  mat = cyclus::NewBlankMaterial(batch_size + (1 + cyclus::eps()));
  TestReserveBatches(mat, in_c1, 3, 1 + cyclus::eps());

  mat = cyclus::NewBlankMaterial(batch_size - (1 + cyclus::eps()));
  TestReserveBatches(mat, in_c1, 4, 0);

  mat = cyclus::NewBlankMaterial(1 + cyclus::eps());
  TestReserveBatches(mat, in_c1, 4, 1 + cyclus::eps());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(BatchReactorTest, BatchInOut) {
  using cyclus::Material;

  EXPECT_THROW(TestBatchIn(1, 0), cyclus::Error);

  Material::Ptr mat = cyclus::NewBlankMaterial(batch_size);
  TestReserveBatches(mat, in_c1, 1, 0);
  TestBatchIn(1, 0);

  mat = cyclus::NewBlankMaterial(batch_size * 2);
  TestReserveBatches(mat, in_c1, 2, 0);
  TestBatchIn(2, 1);

  TestBatchOut(1, 1);
  TestBatchOut(0, 2);

  EXPECT_THROW(TestBatchOut(1, 0), cyclus::Error);
}

}  // namespace cycamore

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* BatchReactorConstructor(cyclus::Context* ctx) {
  return new cycamore::BatchReactor(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// INSTANTIATE_TEST_CASE_P(BatchReactor, FacilityTests,
//                         Values(&BatchReactorConstructor));
INSTANTIATE_TEST_CASE_P(BatchReactor, AgentTests,
                        Values(&BatchReactorConstructor));
