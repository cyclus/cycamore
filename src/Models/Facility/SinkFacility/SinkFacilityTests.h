// SinkFacilityTests.cpp
#include <gtest/gtest.h>

#include "SinkFacility.h"

#include "FacilityModelTests.h"
#include "ModelTests.h"
#include "TestMarket.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SinkFacilityModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* SinkFacilityConstructor(){
  return dynamic_cast<cyclus::FacilityModel*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SinkFacilityTest : public ::testing::Test {
protected:
  SinkFacility* src_facility;
  TestMarket* commod_market;
  std::string commod_;
    
  virtual void SetUp();
  virtual void TearDown();
  void initParameters();
  void setUpSinkFacility();
};
