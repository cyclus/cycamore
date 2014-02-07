#include "manager_inst_tests.h"
#include "inst_model_tests.h"
#include "model_tests.h"
// @MJGFlag - note that I could not link to the commodity test helper
// class here... constructor and destructor were undefined
// for posterity -
// CMakeFiles/CycamoreUnitTestDriver.dir/Models/Inst/ManagerInst/manager_inst_tests.cc.o: In function `TestProducer::TestProducer()':
// manager_inst_tests.cc:(.text+0x80): undefined reference to `cyclus::CommodityTestHelper::cyclus::CommodityTestHelper()'
//   manager_inst_tests.cc:(.text+0xd1): undefined reference to `cyclus::CommodityTestHelper::~cyclus::CommodityTestHelper()'
// manager_inst_tests.cc:(.text+0xe2): undefined reference to `cyclus::CommodityTestHelper::~cyclus::CommodityTestHelper()'
//                                      CMakeFiles/CycamoreUnitTestDriver.dir/Models/Inst/ManagerInst/manager_inst_tests.cc.o: In function `ManagerInstTests::SetUp()':
// manager_inst_tests.cc:(.text+0x259): undefined reference to `cyclus::CommodityTestHelper::cyclus::CommodityTestHelper()'
//                                        CMakeFiles/CycamoreUnitTestDriver.dir/Models/Inst/ManagerInst/manager_inst_tests.cc.o: In function `ManagerInstTests::TearDown()':
// manager_inst_tests.cc:(.text+0x344): undefined reference to `cyclus::CommodityTestHelper::~cyclus::CommodityTestHelper()'


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TestProducer::TestProducer(cyclus::Context* ctx)
    : cyclus::FacilityModel(ctx),
      cyclus::Model(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TestProducer::~TestProducer() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInstTests::SetUp() {
  ctx_ = new cyclus::Context(&ti_, &rec_);
  src_inst = new cycamore::ManagerInst(ctx_);
  producer = new TestProducer(ctx_);
  commodity = cyclus::Commodity("commod");
  capacity = 5;
  producer->AddCommodity(commodity);
  producer->SetCapacity(commodity, capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInstTests::TearDown() {
  delete ctx_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* ManagerInstModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new cycamore::ManagerInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::InstModel* ManagerInstConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::InstModel*>(new cycamore::ManagerInst(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ManagerInstTests, producerexists) {
  using std::set;
  ctx_->AddPrototype("foop", producer);
  src_inst->RegisterAvailablePrototype("foop");
  set<cyclus::CommodityProducer*>::iterator it;
  for (it = src_inst->BeginningProducer(); it != src_inst->EndingProducer();
       it++) {
    EXPECT_EQ(dynamic_cast<TestProducer*>(*it)->name(), producer->name());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ManagerInstTests, productioncapacity) {
  EXPECT_EQ(src_inst->TotalProductionCapacity(commodity), 0);
  src_inst->RegisterCloneAsBuilt(producer);
  EXPECT_EQ(src_inst->TotalProductionCapacity(commodity), capacity);
  src_inst->RegisterCloneAsDecommissioned(producer);
  EXPECT_EQ(src_inst->TotalProductionCapacity(commodity), 0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(ManagerInst, InstModelTests,
                        Values(&ManagerInstConstructor));
INSTANTIATE_TEST_CASE_P(ManagerInst, ModelTests,
                        Values(&ManagerInstModelConstructor));

