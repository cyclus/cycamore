// BatchReactorTests.cpp
#include <gtest/gtest.h>

#include "BatchReactor.h"
#include "FacilityModelTests.h"
#include "ModelTests.h"
#include <string>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* BatchReactorModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new BatchReactor());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::FacilityModel* BatchReactorConstructor(){
  return dynamic_cast<cyclus::FacilityModel*>(new BatchReactor());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BatchReactorTest : public ::testing::Test {
protected:
  
  BatchReactor* src_facility;
  int lencycle, nbatch;
  double in_loadcore, out_loadcore;
  std::string in_commod, in_recipe, out_commod, out_recipe;
  TestMarket* incommod_market;
  TestMarket* outcommod_market;
  std::string commodity;
  double capacity, cost;
  
  virtual void SetUp();
  virtual void TearDown();
  void initSrcFacility();
  void initWorld();
};
