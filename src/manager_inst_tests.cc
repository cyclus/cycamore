#include "manager_inst_tests.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TestProducer::TestProducer(cyclus::Context* ctx) : cyclus::Facility(ctx) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TestProducer::~TestProducer() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInstTests::SetUp() {
  ctx_ = new cyclus::Context(&ti_, &rec_);
  src_inst = new cycamore::ManagerInst(ctx_);
  producer = new TestProducer(ctx_);
  commodity = cyclus::toolkit::Commodity("commod");
  capacity = 5;
  producer->cyclus::toolkit::CommodityProducer::Add(commodity);
  producer->SetCapacity(commodity, capacity);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInstTests::TearDown() {
  delete producer;
  delete src_inst;
  delete ctx_;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* ManagerInstitutionConstructor(cyclus::Context* ctx) {
  return new cycamore::ManagerInst(ctx);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ManagerInstTests, producerexists) {
  using std::set;
  ctx_->AddPrototype("foop", producer);
  set<cyclus::toolkit::CommodityProducer*>::iterator it;
  for (it = src_inst->cyclus::toolkit::CommodityProducerManager::
          producers().begin();
       it != src_inst->cyclus::toolkit::CommodityProducerManager::
          producers().end();
       it++) {
    EXPECT_EQ(dynamic_cast<TestProducer*>(*it)->prototype(),
              producer->prototype());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ManagerInstTests, productioncapacity) {
  EXPECT_EQ(src_inst->TotalCapacity(commodity), 0);
  src_inst->BuildNotify(producer);
  EXPECT_EQ(src_inst->TotalCapacity(commodity), capacity);
  src_inst->DecomNotify(producer);
  EXPECT_EQ(src_inst->TotalCapacity(commodity), 0);
}

TEST_F(ManagerInstTests, PositionDefault) {
  std::string config = 
     "<prototypes>  <val>foobar</val> </prototypes>";

  int simdur = 5;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:ManagerInst"), config, simdur);
  sim.DummyProto("foobar");
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);
}

TEST_F(ManagerInstTests, PositionInitialize) {
  std::string config = 
     "<prototypes>  <val>foobar</val> </prototypes>"
     "<longitude>   -20.0             </longitude>"
     "<latitude>    2.0               </latitude>";

  int simdur = 5;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:ManagerInst"), config, simdur);
  sim.DummyProto("foobar");
  int id = sim.Run();

  cyclus::QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 2.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), -20.0);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(ManagerInst, InstitutionTests,
                        Values(&ManagerInstitutionConstructor));
INSTANTIATE_TEST_CASE_P(ManagerInst, AgentTests,
                        Values(&ManagerInstitutionConstructor));
