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

//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ManagerInstTests, BasicMetadata) {
  // this tests verifies the initialization of the latitude variable
  
  std::string config = 
    "<prototypes>  <val>foobar</val> </prototypes>"
    "<build_times> <val>1</val>      </build_times>"
    "<n_build>     <val>3</val>      </n_build>"
    "<lifetimes>   <val>2</val>      </lifetimes>"
    " "
    " "
    "   <metadata>"
    "     <item> "
    "       <key>string_key</key>"
    "       <value>string_value%s</value>"
    "     </item> "
    "     <item> "
    "       <key>double_key</key>"
    "       <value>0.01254%d</value>"
    "     </item> "
    "     <item> "
    "       <key>int_key</key>"
    "       <value>-1254%i</value>"
    "     </item> "
    "     <item> "
    "       <key>uint_key</key>"
    "       <value>1254%u</value>"
    "     </item> "
    "     <item> "
    "       <key>bool_key</key>"
    "       <value>true%b</value>"
    "     </item> "
    "   </metadata>";

  int simdur = 5;
	cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:ManagerInst"), config, simdur);
	sim.DummyProto("foobar");
	int id = sim.Run();

  std::vector<cyclus::Cond> conds;
  cyclus::QueryResult qr; 
  conds.push_back(cyclus::Cond("keyword", "==", std::string("string_key")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "string_value");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "string");
  
  conds[0] = cyclus::Cond("keyword", "==", std::string("double_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "0.012540");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "double");
  
  conds[0] = cyclus::Cond("keyword", "==", std::string("int_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "-1254");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "int");
  
  conds[0] = cyclus::Cond("keyword", "==", std::string("uint_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "1254");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "uint");
  
  conds[0] = cyclus::Cond("keyword", "==", std::string("bool_key"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "true");
  EXPECT_EQ(qr.GetVal<std::string>("Type"), "bool");
}


TEST_F(ManagerInstTests, UsageMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config = 
    "<prototypes>  <val>foobar</val> </prototypes>"
    "<build_times> <val>1</val>      </build_times>"
    "<n_build>     <val>3</val>      </n_build>"
    "<lifetimes>   <val>2</val>      </lifetimes>"
    "   "
    "   "
    "   <usagemetadata>"
    "     <item> "
    "       <keyword>co2</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>decommission</key> "
    "           <value>25</value> "
    "         </item> "
    "         <item> "
    "           <key>deployment</key> "
    "           <value>45</value> "
    "         </item> "
    "         <item> "
    "           <key>timestep</key> "
    "           <value>35</value> "
    "         </item> "
    "         <item> "
    "           <key>throughput</key> "
    "           <value>15</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>water</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>deployment</key> "
    "           <value>43</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>land</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>decommission</key> "
    "           <value>24</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>manpower</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>timestep</key> "
    "           <value>32</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   "
    "     <item> "
    "       <keyword>lolipop</keyword>"
    "       <usage> "
    "         <item> "
    "           <key>throughput</key> "
    "           <value>11</value> "
    "         </item> "
    "       </usage> "
    "     </item> "
    "   </usagemetadata>";

  int simdur = 5;
	cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:ManagerInst"), config, simdur);
	sim.DummyProto("foobar");
	int id = sim.Run();

  std::vector<cyclus::Cond> conds;
  cyclus::QueryResult qr; 
  conds.push_back(cyclus::Cond("keyword", "==", std::string("co2")));
  conds.push_back(cyclus::Cond("Type", "==", std::string("decommission")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "25.000000");
  conds.clear();
  conds.push_back(cyclus::Cond("keyword", "==", std::string("co2")));
  conds.push_back(cyclus::Cond("Type", "==", std::string("deployment")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "45.000000");
  conds[1] = cyclus::Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "35.000000");
  conds[1] = cyclus::Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "15.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("water"));
  conds[1] = cyclus::Cond("Type", "==", std::string("deployment"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "43.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("land"));
  conds[1] = cyclus::Cond("Type", "==", std::string("decommission"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "24.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("manpower"));
  conds[1] = cyclus::Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "32.000000");

  conds[0] = cyclus::Cond("keyword", "==", std::string("lolipop"));
  conds[1] = cyclus::Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "11.000000");
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
