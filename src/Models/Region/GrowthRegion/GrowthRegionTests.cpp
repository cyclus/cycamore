// GrowthRegionTests.cpp
#include "GrowthRegionTests.h"

#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "RegionModelTests.h"
#include "ModelTests.h"
#include "InputXML.h"
#include "StubModel.h"
#include "BuildingManager.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::SetUp() {
  reg_ = new GrowthRegion();
  new_region_ = new GrowthRegion();
  inst_ = new ManagerInst();
  child1_ = new PlayerChild();
  child2_ = new PlayerChild();
  initSupplyDemand();
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::TearDown() {
  delete child1_;
  delete child2_;
  delete inst_;
  delete reg_;
  delete new_region_;
  delete p1_;
  delete p2_;
  delete commodity_;
}  

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
xmlDocPtr GrowthRegionTest::getXMLDoc() {  
  stringstream ss("");
  ss <<
    "<?xml version=\"1.0\"?>\n" <<
    "<document>\n" <<
    "  <name>power</name>\n" <<
    "  <demand>\n" << 
    "    <type>exp</type>\n" <<
    "    <parameters>5 0.0005 " << power_demand_ << "</parameters>\n" <<
    "    <metby>\n" <<
    "      <facility>" << producer1_name_ << "</facility>\n" <<
    "      <capacity>" << producer1_capacity_ << "</capacity>\n" <<
    "    </metby>\n"<<
    "    <metby>\n" <<
    "      <facility>" << producer2_name_ << "</facility>\n" <<
    "      <capacity>" << producer2_capacity_ << "</capacity>\n" <<
    "    </metby>\n" <<
    "  </demand>\n" <<
    "</document>";
  
  string snippit = ss.str();

  return xmlParseMemory(snippit.c_str(),snippit.size());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int GrowthRegionTest::containerSizes(GrowthRegion* reg) {
  int sizes = reg_->builders_.size() + reg_->producers_.size() +
    reg_->commodities_.size();
  return sizes;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Producer GrowthRegionTest::getProducer(GrowthRegion* reg, int i) {  
  xmlDocPtr doc = getXMLDoc();
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  xmlNodePtr node = doc->children;
  
  string name = 
    (const char*)XMLinput->get_xpath_content(context,node,"name");
  Commodity commodity(name);

  xmlNodeSetPtr producer_nodes = 
    XMLinput->get_xpath_elements(context,node,"demand/metby");
  xmlNodePtr pnode = producer_nodes->nodeTab[i];
  return reg->getProducer(context,pnode,commodity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::initSupplyDemand() {
  power_demand_ = 1000;
  producer1_capacity_ = 800;
  producer1_cost_ = producer1_capacity_;
  producer1_name_ = "ReactorA";
  producer2_capacity_ = 200;
  producer2_cost_ = producer2_capacity_;
  producer2_name_ = "ReactorB";
  commodity_name_ = "power";
  commodity_ = new Commodity(commodity_name_);
  p1_ = new Producer(producer1_name_,*commodity_,producer1_capacity_,
                     producer1_cost_);
  p2_ = new Producer(producer2_name_,*commodity_,producer2_capacity_,
                     producer2_cost_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::initPlayers() {
  inst_->setCommodity(*commodity_);

  child1_->registerCommodity(*commodity_);
  child1_->setProductionCapacity(producer1_capacity_,*commodity_);
  child1_->registerManager(inst_,*commodity_);

  child2_->registerCommodity(*commodity_);
  child2_->setProductionCapacity(producer2_capacity_,*commodity_);
  child2_->registerManager(inst_,*commodity_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::setUpChildren() {
  child1_->setName(producer1_name_);
  inst_->addChild(child1_);
  child1_->doSetParent(inst_);
  child2_->setName(producer2_name_);
  inst_->addChild(child2_);
  child2_->doSetParent(inst_);
  reg_->addChild(inst_);
  inst_->doSetParent(reg_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::doInit() {
  xmlDocPtr doc = getXMLDoc();
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  xmlNodePtr node = doc->children;

  reg_->initCommodity(node,context);
  setUpChildren();  
  initPlayers();
  reg_->initPlayerManagers();
  reg_->initBuildManager();
  reg_->populateProducerMaps();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::testProducerInit() {
  // make test objects
  Producer p = getProducer(reg_,0);
  // test the objects
  EXPECT_EQ(p.name(),p1_->name());
  EXPECT_EQ(p.commodity(),p1_->commodity());
  EXPECT_EQ(p.capacity(),p1_->capacity());
  EXPECT_EQ(p.cost(),p1_->cost());
  EXPECT_EQ(p,*p1_);
  // quick test other producer
  p = getProducer(reg_,1);
  EXPECT_EQ(p,*p2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::testCommodityInit() {  
  xmlDocPtr doc = getXMLDoc();
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  xmlNodePtr node = doc->children;

  EXPECT_NO_THROW(reg_->initCommodity(node,context));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::testProducerNames() {
    map<string,Producer*> producer_names;
    reg_->populateProducerNames(*commodity_,producer_names);
    map<string,Producer*>::iterator it;
    for (it = producer_names.begin(); 
         it != producer_names.end(); it++) {
      EXPECT_EQ(it->first,it->second->name());
    }
    EXPECT_EQ(*producer_names[p1_->name()],*p1_);
    EXPECT_EQ(*producer_names[p2_->name()],*p2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::testMapsInit() {
  string name = "region";
  reg_->setName(name);
  setUpChildren();
  map<string,Producer*> producer_names;
  reg_->populateProducerNames(*commodity_,producer_names);
  reg_->populateMaps(reg_,producer_names);
  //cout << reg_->printMaps();
  //cout << reg_->printChildren();
  EXPECT_EQ(reg_->builders_[producer_names[producer1_name_]],inst_);
  EXPECT_EQ(reg_->builders_[producer_names[producer2_name_]],inst_);
  EXPECT_EQ(reg_->producers_[producer_names[producer1_name_]],child1_);
  EXPECT_EQ(reg_->producers_[producer_names[producer2_name_]],child2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::testBuildDecision() {
  doInit();
  double supply = reg_->sdmanager_.supply(*commodity_);
  double demand = reg_->sdmanager_.demand(*commodity_,0);
  double unmet_demand = demand - supply;
  vector<BuildOrder> orders = 
    reg_->buildmanager_->makeBuildDecision(*commodity_,unmet_demand);
  EXPECT_EQ(orders.at(0).number,1);
  EXPECT_EQ(*orders.at(0).producer,*p1_);
  EXPECT_EQ(orders.at(1).number,2);
  EXPECT_EQ(*orders.at(1).producer,*p2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::enterMarkets() {
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),0);
  child1_->enterMarket(*commodity_);
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),producer1_capacity_);
  child2_->enterMarket(*commodity_);
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),
            producer1_capacity_ + producer2_capacity_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::leaveMarkets() {
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),
            producer1_capacity_ + producer2_capacity_);
  child1_->leaveMarket(*commodity_);
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),producer2_capacity_);
  child2_->leaveMarket(*commodity_);
  EXPECT_EQ(reg_->sdmanager_.supply(*commodity_),0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* GrowthRegionModelConstructor(){
  return dynamic_cast<Model*>(new GrowthRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RegionModel* GrowthRegionConstructor(){
  return dynamic_cast<RegionModel*>(new GrowthRegion());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTest, CopyFreshModel) {
  // deep copy
  EXPECT_NO_THROW(new_region_->copyFreshModel(dynamic_cast<Model*>(reg_))); 
  // still a build region
  EXPECT_NO_THROW(dynamic_cast<GrowthRegion*>(new_region_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTest,TestInitParts) {
  EXPECT_EQ(containerSizes(reg_),0);
  testProducerInit();
  testCommodityInit();
  testProducerNames();
  testMapsInit();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTest,TestInitFull) {
  EXPECT_NO_THROW(doInit());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTest,TestPlayerInteraction) {
  doInit();
  enterMarkets();
  leaveMarkets();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(GrowthRegionTest,TestBuildDecision) {
  testBuildDecision();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

