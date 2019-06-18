#include <sstream>

#include "growth_region_tests.h"
#if CYCLUS_HAS_COIN

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::SetUp() {
  ctx = new cyclus::Context(&ti, &rec);
  region = new cycamore::GrowthRegion(ctx);
  commodity_name = "commod";
  demand_type = "linear";
  demand_params = "5 5";
  demand_start = "0";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTests::TearDown() {
  delete region;
  delete ctx;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool GrowthRegionTests::ManagesCommodity(cyclus::toolkit::Commodity& commodity) {
  return region->sdmanager()->ManagesCommodity(commodity);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests, init) {
  cyclus::toolkit::Commodity commodity(commodity_name);
  cyclus::toolkit::ExpFunctionFactory eff;
  region->sdmanager()->RegisterCommodity(commodity, eff.GetFunctionPtr("2.0 4.0"));
  EXPECT_TRUE(ManagesCommodity(commodity));
}

//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTests, BasicMetadata) {
  // this tests verifies the initialization of the latitude variable
  
  std::string config = 
    " <prototypes>  <val>foobar</val> </prototypes> " 
	  " <growth> " 
	  "   <commod>test_commod</commod> " 
	  "   <piecewise_function> " 
	  "     <piece> " 
	  "       <start>0</start> " 
	  "       <function> " 
	  "         <type>linear</type> " 
	  "         <params>1 2</params> " 
	  "       </function> " 
	  "     </piece> " 
	  "   </piecewise_function> " 
	  " </growth> " 
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
	cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:GrowthRegion"), config, simdur);
	sim.DummyProto("foobar");
	int id = sim.Run();

  std::vector<cyclus::Cond> conds;
  QueryResult qr; 
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


TEST_F(GrowthRegionTests, UsageMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config = 
    " <prototypes>  <val>foobar</val> </prototypes> " 
	  " <growth> " 
	  "   <commod>test_commod</commod> " 
	  "   <piecewise_function> " 
	  "     <piece> " 
	  "       <start>0</start> " 
	  "       <function> " 
	  "         <type>linear</type> " 
	  "         <params>1 2</params> " 
	  "       </function> " 
	  "     </piece> " 
	  "   </piecewise_function> " 
	  " </growth> " 
    "   "
    "   "
    "   <usagemetadata> "
    "     <item> "
    "       <keyword>co2</keyword> "
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
	cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:GrowthRegion"), config, simdur);
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
}  // namespace cycamore

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* GrowthRegionConstructor(cyclus::Context* ctx) {
  return new cycamore::GrowthRegion(ctx);
}

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionTests,
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, AgentTests,
                        Values(&GrowthRegionConstructor));
#endif  // CYCLUS_HAS_COIN
