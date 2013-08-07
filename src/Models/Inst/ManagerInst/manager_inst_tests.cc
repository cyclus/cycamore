#include "manager_inst_tests.h"
#include "InstModelTests.h"
#include "ModelTests.h"
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

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TestProducer::TestProducer() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TestProducer::~TestProducer() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void ManagerInstTests::SetUp()
{
  src_inst = new ManagerInst();
  producer = new TestProducer();
  commodity = cyclus::Commodity("commod");
  capacity = 5;
  producer->addCommodity(commodity);
  producer->setCapacity(commodity,capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void ManagerInstTests::TearDown()
{
  delete producer;
  delete src_inst;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::Model* ManagerInstModelConstructor()
{
  return dynamic_cast<cyclus::Model*>(new ManagerInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::InstModel* ManagerInstConstructor()
{
  return dynamic_cast<cyclus::InstModel*>(new ManagerInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(ManagerInstTests,producerexists) 
{
  src_inst->registerAvailablePrototype(producer);
  set<cyclus::SupplyDemand::CommodityProducer*>::iterator it;
  for (it = src_inst->beginningProducer(); it != src_inst->endingProducer(); it++) 
    {
      EXPECT_EQ((*it),producer);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(ManagerInstTests,productioncapacity) 
{
  EXPECT_EQ(src_inst->totalProductionCapacity(commodity),0);
  src_inst->registerCloneAsBuilt(producer);
  EXPECT_EQ(src_inst->totalProductionCapacity(commodity),capacity);
  src_inst->registerCloneAsDecommissioned(producer);
  EXPECT_EQ(src_inst->totalProductionCapacity(commodity),0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(ManagerInst, InstModelTests, Values(&ManagerInstConstructor));
INSTANTIATE_TEST_CASE_P(ManagerInst, ModelTests, Values(&ManagerInstModelConstructor));

