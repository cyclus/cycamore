// GrowthRegionTests.cpp
#include <gtest/gtest.h>
#include "GrowthRegion.h"
using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class GrowthRegionTest : public ::testing::Test {
protected:
  GrowthRegion* reg_;
  GrowthRegion* new_region_; 
  
  virtual void SetUp(){
    reg_ = new GrowthRegion();
    new_region_ = new GrowthRegion();
  };
  
  virtual void TearDown() {
    delete reg_;
    delete new_region_;
  }  
};

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

