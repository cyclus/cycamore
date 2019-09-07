#include "separations.h"

#include <gtest/gtest.h>
#include <sstream>
#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::CompMap;
using cyclus::Material;
using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::toolkit::MatQuery;

namespace cycamore {

TEST(SeparationsTests, SepMaterial) {
  CompMap comp;
  comp[id("U235")] = 10;
  comp[id("U238")] = 90;
  comp[id("Pu239")] = 1;
  comp[id("Pu240")] = 2;
  comp[id("Am241")] = 3;
  comp[id("Am242")] = 2.8;
  double qty = 100;
  Composition::Ptr c = Composition::CreateFromMass(comp);
  Material::Ptr mat = Material::CreateUntracked(qty, c);

  std::map<int, double> effs;
  effs[id("U")] = .7;
  effs[id("Pu")] = .4;
  effs[id("Am241")] = .4;

  Material::Ptr sep = SepMaterial(effs, mat);
  MatQuery mqorig(mat);
  MatQuery mqsep(sep);

  EXPECT_DOUBLE_EQ(effs[id("U")] * mqorig.mass("U235"), mqsep.mass("U235"));
  EXPECT_DOUBLE_EQ(effs[id("U")] * mqorig.mass("U238"), mqsep.mass("U238"));
  EXPECT_DOUBLE_EQ(effs[id("Pu")] * mqorig.mass("Pu239"), mqsep.mass("Pu239"));
  EXPECT_DOUBLE_EQ(effs[id("Pu")] * mqorig.mass("Pu240"), mqsep.mass("Pu240"));
  EXPECT_DOUBLE_EQ(effs[id("Am241")] * mqorig.mass("Am241"), mqsep.mass("Am241"));
  EXPECT_DOUBLE_EQ(0, mqsep.mass("Am242"));
}


// Check that cumulative separations efficiency for a single nuclide of less than or equal to one does not trigger an error.
TEST(SeparationsTests, SeparationEfficiency) {

  int simdur = 2;
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>Pu239</comp> <eff>.2</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>Pu239</comp> <eff>.1</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>Pu239</comp> <eff>.5</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
  ;


  cyclus::MockSim sim1(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);

  EXPECT_NO_THROW(sim1.Run()) << "Cumulative separation efficiency smaler than 1 is throwing an error but should not.";


  config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.1</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.2</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.2</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.1</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.7</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.5</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
      ;


  cyclus::MockSim sim2(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);

  EXPECT_NO_THROW(sim2.Run()) << "Cumulative separation efficiency of 1 is throwing an error but should not.";
}

// Check that an error is correctly thrown when separations efficiency of greater than one.
TEST(SeparationsTests, SeparationEfficiencyThrowing) {
  int simdur = 2;

  // Check that single separations efficiency for a single nuclide of greater than one does not trigger an error.
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>1.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
      ;

  cyclus::MockSim sim1(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);

  EXPECT_THROW(sim1.Run(), cyclus::ValueError) << "Direct separation efficiency greater than 1 is not throwing an error but should.";

// Check if a cumulative separation efficiency greater than 1 for a unique nuclide throw an error as expected.
  config =
    "<streams>"
    "    <item>"
    "        <commod>stream1</commod>"
    "        <info>"
    "            <buf_size>-1</buf_size>"
    "            <efficiencies>"
    "                <item><comp>U</comp> <eff>0.6</eff></item>"
    "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
    "            </efficiencies>"
    "        </info>"
    "    </item>"
    "    <item>"
    "        <commod>stream2</commod>"
    "        <info>"
    "            <buf_size>-1</buf_size>"
    "            <efficiencies>"
    "                <item><comp>U</comp> <eff>0.1</eff></item>"
    "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
    "            </efficiencies>"
    "        </info>"
    "    </item>"
    "</streams>"
    ""
    "<leftover_commod>waste</leftover_commod>"
    "<throughput>100</throughput>"
    "<feedbuf_size>100</feedbuf_size>"
    "<feed_commods> <val>feed</val> </feed_commods>"
    ;

  cyclus::MockSim sim2(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);

  EXPECT_THROW(sim2.Run(), cyclus::ValueError) << "Single cumulative separation efficiency greater than 1 is not throwing an error but should.";
// Check if a cumulative separation efficiency greater than 1 for multiple nuclides throw an error as expected.
  config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "    <item>"
      "        <commod>stream2</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
      ;

  cyclus::MockSim sim3(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);

  EXPECT_THROW(sim3.Run(), cyclus::ValueError) << "Multiple cumulative separation efficiencies greater than 1 are not throwing an error but should.";
}

TEST(SeparationsTests, SepMixElemAndNuclide) {
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
     ;

  CompMap m;
  m[id("u235")] = 0.08;
  m[id("u238")] = 0.9;
  m[id("Pu239")] = .01;
  m[id("Pu240")] = .01;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddRecipe("recipe1", c);
  int id = sim.Run();

  std::vector<Cond> conds;
  conds.push_back(Cond("SenderId", "==", id));
  int resid = sim.db().Query("Transactions", &conds).GetVal<int>("ResourceId");
  MatQuery mq (sim.GetMaterial(resid));
  EXPECT_DOUBLE_EQ(m[922350000]*0.6*100, mq.mass("U235"));
  EXPECT_DOUBLE_EQ(m[922380000]*0.6*100, mq.mass("U238"));
  EXPECT_DOUBLE_EQ(m[942390000]*0.7*100, mq.mass("Pu239"));
  EXPECT_DOUBLE_EQ(0, mq.mass("Pu240"));
  
  // checking the write amount of SWU has been repported
  conds.clear();
  conds.push_back(Cond("Time", "==", 1));
  QueryResult qr = sim.db().Query("TimeSeriesThroughput", &conds);
  EXPECT_NEAR(qr.GetVal<double>("Value"), 1, 0.01);
}

TEST(SeparationsTests, Retire) {
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U235</comp> <eff>1.0</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
     ;

  CompMap m;
  m[id("u235")] = 0.1;
  m[id("u238")] = 0.9;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 5;
  int life = 2;

  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"),
          config, simdur, life);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddSink("waste").capacity(70).Finalize();
  sim.AddRecipe("recipe1", c);
  int id = sim.Run();

  // Separations should stop requesting material at its lifetime
  // (it is smart enough to not request material on its last timestep because
  //  it knows it won't be able to process it)
  std::vector<Cond> conds;
  conds.push_back(Cond("ReceiverId", "==", id));
  QueryResult qr = sim.db().Query("Transactions", &conds);
  EXPECT_EQ(life - 1, qr.rows.size())
      << "failed to stop ordering near retirement";

  // Separations should discharge all material before decomissioning
  conds.clear();
  conds.push_back(Cond("SenderId", "==", id));
  qr = sim.db().Query("Transactions", &conds);
  double tot_mat = 0;
  for (int i = 0; i < qr.rows.size(); i++) {
    cyclus::Material::Ptr m = sim.GetMaterial(qr.GetVal<int>("ResourceId", i));
    tot_mat += m->quantity();
  }
  EXPECT_EQ(100, tot_mat)
    << "total material traded away does not equal total material separated";
  EXPECT_EQ(3.0, qr.rows.size())
      << "failed to discharge all material before decomissioning";
 }

 TEST(SeparationsTests, PositionInitialize) {
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>";
  CompMap m;
  m[id("u235")] = 0.08;
  m[id("u238")] = 0.9;
  m[id("Pu239")] = .01;
  m[id("Pu240")] = .01;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddRecipe("recipe1", c);
  int id = sim.Run();

  QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 0.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 0.0);
 }

  TEST(SeparationsTests, PositionInitialize2) {
  std::string config =
      "<streams>"
      "    <item>"
      "        <commod>stream1</commod>"
      "        <info>"
      "            <buf_size>-1</buf_size>"
      "            <efficiencies>"
      "                <item><comp>U</comp> <eff>0.6</eff></item>"
      "                <item><comp>Pu239</comp> <eff>.7</eff></item>"
      "            </efficiencies>"
      "        </info>"
      "    </item>"
      "</streams>"
      ""
      "<leftover_commod>waste</leftover_commod>"
      "<throughput>100</throughput>"
      "<feedbuf_size>100</feedbuf_size>"
      "<feed_commods> <val>feed</val> </feed_commods>"
      "<latitude>10.0</latitude> "
      "<longitude>15.0</longitude> ";
  CompMap m;
  m[id("u235")] = 0.08;
  m[id("u238")] = 0.9;
  m[id("Pu239")] = .01;
  m[id("Pu240")] = .01;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddRecipe("recipe1", c);
  int id = sim.Run();

  QueryResult qr = sim.db().Query("AgentPosition", NULL);
  EXPECT_EQ(qr.GetVal<double>("Latitude"), 10.0);
  EXPECT_EQ(qr.GetVal<double>("Longitude"), 15.0);
 }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST(SeparationsTests, StringMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config =
    "  <streams>"
    "      <item>"
    "          <commod>stream1</commod>"
    "          <info>"
    "              <buf_size>-1</buf_size>"
    "              <efficiencies>"
    "                  <item><comp>U</comp> <eff>0.6</eff></item>"
    "                  <item><comp>Pu239</comp> <eff>.7</eff></item>"
    "              </efficiencies>"
    "          </info>"
    "      </item>"
    "  </streams>"
    "  <leftover_commod>waste</leftover_commod>"
    "  <throughput>100</throughput>"
    "  <feedbuf_size>100</feedbuf_size>"
    "  <feed_commods> <val>feed</val> </feed_commods>"
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

  CompMap m;
  m[id("u235")] = 0.08;
  m[id("u238")] = 0.9;
  m[id("Pu239")] = .01;
  m[id("Pu240")] = .01;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddRecipe("recipe1", c);
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



TEST(SeparationsTests, UsageMetadata) {
  // this tests verifies the initialization of the latitude variable

  std::string config =
    "  <streams>"
    "      <item>"
    "          <commod>stream1</commod>"
    "          <info>"
    "              <buf_size>-1</buf_size>"
    "              <efficiencies>"
    "                  <item><comp>U</comp> <eff>0.6</eff></item>"
    "                  <item><comp>Pu239</comp> <eff>.7</eff></item>"
    "              </efficiencies>"
    "          </info>"
    "      </item>"
    "  </streams>"
    "  <leftover_commod>waste</leftover_commod>"
    "  <throughput>100</throughput>"
    "  <feedbuf_size>100</feedbuf_size>"
    "  <feed_commods> <val>feed</val> </feed_commods>"
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

  CompMap m;
  m[id("u235")] = 0.08;
  m[id("u238")] = 0.9;
  m[id("Pu239")] = .01;
  m[id("Pu240")] = .01;
  Composition::Ptr c = Composition::CreateFromMass(m);

  int simdur = 2;
  cyclus::MockSim sim(cyclus::AgentSpec(":cycamore:Separations"), config, simdur);
  sim.AddSource("feed").recipe("recipe1").Finalize();
  sim.AddSink("stream1").capacity(100).Finalize();
  sim.AddRecipe("recipe1", c);
  int id = sim.Run();

  std::vector<Cond> conds;
  QueryResult qr;
  conds.push_back(Cond("keyword", "==", std::string("co2")));
  conds.push_back(Cond("Type", "==", std::string("decommission")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "25.000000");
  conds.clear();
  conds.push_back(Cond("keyword", "==", std::string("co2")));
  conds.push_back(Cond("Type", "==", std::string("deployment")));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "45.000000");
  conds[1] = Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "35.000000");
  conds[1] = Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "15.000000");

  conds[0] = Cond("keyword", "==", std::string("water"));
  conds[1] = Cond("Type", "==", std::string("deployment"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "43.000000");

  conds[0] = Cond("keyword", "==", std::string("land"));
  conds[1] = Cond("Type", "==", std::string("decommission"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "24.000000");

  conds[0] = Cond("keyword", "==", std::string("manpower"));
  conds[1] = Cond("Type", "==", std::string("timestep"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "32.000000");

  conds[0] = Cond("keyword", "==", std::string("lolipop"));
  conds[1] = Cond("Type", "==", std::string("throughput"));
  qr = sim.db().Query("Metadata", &conds);
  EXPECT_EQ(qr.GetVal<std::string>("Value"), "11.000000");
}
} // namespace cycamore

