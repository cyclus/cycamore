// SourceFacilityTests.cpp
#include <gtest/gtest.h>

#include "SourceFacility.h"

#include "FacilityModelTests.h"
#include "ModelTests.h"
#include "TestMarket.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::Model* SourceFacilityModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new SourceFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::FacilityModel* SourceFacilityConstructor(){
  return dynamic_cast<cyclus::FacilityModel*>(new SourceFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class SourceFacilityTest : public ::testing::Test {
protected:
  SourceFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_, recipe_name_;
  CompMapPtr recipe_;
    
  virtual void SetUp();
  virtual void TearDown();
  void initParameters();
  void setUpSourceFacility();
};
