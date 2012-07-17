// GrowthRegionTests.cpp
#include "GrowthRegionTests.h"

#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "RegionModelTests.h"
#include "ModelTests.h"
#include "InputXML.h"
#include "StubModel.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::SetUp() {
  reg_ = new GrowthRegion();
  new_region_ = new GrowthRegion();
  child1_ = new StubModel();
  child2_ = new StubModel();
  initProducers();
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::TearDown() {
  delete reg_;
  delete new_region_;
  delete child1_;
  delete child2_;
  delete p1_;
  delete p2_;
  delete commodity_;
}  

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int GrowthRegionTest::containerSizes(GrowthRegion* reg) {
  int sizes = reg_->builders_.size() + reg_->producers_.size() +
    reg_->commodities_.size();
  return sizes;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
xmlDocPtr GrowthRegionTest::getXMLDoc() {  
  stringstream ss("");
  ss <<
    "<?xml version=\"1.0\"?>\n" <<
    "<document>\n" <<
    "  <name>power</name>\n" <<
    "  <demand>\n" << 
    "    <type>exponential</type>\n" <<
    "    <parameters>5 0.0005 1800</parameters>\n" <<
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
void GrowthRegionTest::initProducers() {
  producer1_capacity_ = 1050;
  producer1_cost_ = 1;
  producer1_name_ = "ReactorA";
  producer2_capacity_ = 750;
  producer2_cost_ = 1;
  producer2_name_ = "ReactorB";
  commodity_name_ = "power";
  commodity_ = new Commodity(commodity_name_);
  p1_ = new Producer(producer1_name_,*commodity_,producer1_capacity_,
                     producer1_cost_);
  p2_ = new Producer(producer2_name_,*commodity_,producer2_capacity_,
                     producer2_cost_);
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
  // EXPECT_EQ(p,*p2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::testCommodityInit() {  
  xmlDocPtr doc = getXMLDoc();
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  xmlNodePtr node = doc->children;

  EXPECT_NO_THROW(reg_->initCommodity(node,context));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::testMapsInit() {
  string name = "region";
  reg_->setName(name);
  setUpChildren();
  //EXPECT_NO_THROW(reg_->populateProducerMaps());
  // EXPECT_EQ(reg_->builders_[*p1_],reg_);
  // EXPECT_EQ(reg_->builders_[*p2_],reg_);
  // EXPECT_EQ(reg_->producers_[*p1_],child1_);
  // EXPECT_EQ(reg_->producers_[*p2_],child2_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegionTest::setUpChildren() {
  child1_->setName(producer1_name_);
  //reg_->addChild(child1_);
  child1_->doSetParent(reg_);
  child2_->setName(producer2_name_);
  //reg_->addChild(child2_);
  //child2_->doSetParent(reg_);
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
TEST_F(GrowthRegionTest,TestInit) {
  EXPECT_EQ(containerSizes(reg_),0);
  EXPECT_NO_THROW(testProducerInit());
  EXPECT_NO_THROW(testCommodityInit());
  EXPECT_NO_THROW(testMapsInit());
  //EXPECT_NO_THROW(doInit(reg_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

