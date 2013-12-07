// batch_reactor_tests.cc

#include <sstream>

#include "commodity.h"
#include "composition.h"
#include "facility_model_tests.h"
#include "model_tests.h"
#include "model.h"
#include "xml_query_engine.h"

#include "batch_reactor_tests.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactorTest::SetUp() {
  src_facility = new BatchReactor(tc_.get());
  InitParameters();
  SetUpSourceFacility();
  //InitWorld();
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

  cyclus::CompMap v;
  v[92235] = 1;
  v[92238] = 2;
  cyclus::Composition::Ptr recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(in_recipe, recipe);

  v[94239] = 0.25;
  recipe = cyclus::Composition::CreateFromAtom(v);
  tc_.get()->AddRecipe(out_recipe, recipe);
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
  src_facility->AddCommodity(commodity);
  src_facility->cyclus::CommodityProducer::SetCapacity(commodity, capacity);
  src_facility->cyclus::CommodityProducer::SetCost(commodity, capacity);
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

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(src_facility->ProducesCommodity(commod));
  EXPECT_EQ(capacity, src_facility->ProductionCapacity(commod));
  EXPECT_EQ(cost, src_facility->ProductionCost(commod));
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
     << "  <commodity_production>"
     << "    <commodity>" << commodity << "</commodity>"
     << "    <capacity>" << capacity << "</capacity>"
     << "    <cost>" << cost << "</cost>"
     << "  </commodity_production>"
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

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(fac.ProducesCommodity(commod));
  EXPECT_EQ(capacity, fac.ProductionCapacity(commod));
  EXPECT_EQ(cost, fac.ProductionCost(commod));
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

  cyclus::Commodity commod(commodity);
  EXPECT_TRUE(cloned_fac->ProducesCommodity(commod));
  EXPECT_EQ(capacity, cloned_fac->ProductionCapacity(commod));
  EXPECT_EQ(cost, cloned_fac->ProductionCost(commod));
  
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
