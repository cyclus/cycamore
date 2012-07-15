// GrowthRegionTests.cpp
#include "GrowthRegionTests.h"

#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "RegionModelTests.h"
#include "ModelTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::SetUp() {
  reg_ = new GrowthRegion();
  new_region_ = new GrowthRegion();
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::TearDown() {
  delete reg_;
  delete new_region_;
}  

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int GrowthRegionTest::buildersSize(GrowthRegion* reg) {
  return reg_->builders_.size();
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
    "      <facility>ReactorA</facility>\n" <<
    "      <capacity>1050</capacity>\n" <<
    "    </metby>\n"<<
    "    <metby>\n" <<
    "      <facility>ReactorB</facility>\n" <<
    "      <capacity>750</capacity>\n" <<
    "    </metby>\n" <<
    "  </demand>\n" <<
    "</document>";
  
  string snippit = ss.str();

  return xmlParseMemory(snippit.c_str(),snippit.size());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegionTest::doInit(GrowthRegion* reg) {  
  xmlDocPtr doc = getXMLDoc();
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  xmlNodePtr startNode = doc->children;

  reg_->initCommodity(startNode,xpathCtx);
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
TEST_F(GrowthRegionTest, TestPrivateAccess) {
  EXPECT_EQ(buildersSize(reg_),0);
  EXPECT_NO_THROW(doInit(reg_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

