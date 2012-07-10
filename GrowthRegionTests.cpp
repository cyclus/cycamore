// GrowthRegionTests.cpp
#include "GrowthRegionTests.h"

#include <gtest/gtest.h>
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
INSTANTIATE_TEST_CASE_P(GrowthRegion, RegionModelTests, 
                        Values(&GrowthRegionConstructor));
INSTANTIATE_TEST_CASE_P(GrowthRegion, ModelTests, 
                        Values(&GrowthRegionModelConstructor));

