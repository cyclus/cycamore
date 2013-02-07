// SinkFacilityTests.cpp
#include <gtest/gtest.h>

#include "SinkFacility.h"

#include "FacilityModelTests.h"
#include "ModelTests.h"
#include "TestMarket.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* SinkFacilityModelConstructor(){
  return dynamic_cast<Model*>(new SinkFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FacilityModel* SinkFacilityConstructor(){
  return dynamic_cast<FacilityModel*>(new SinkFacility());
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
