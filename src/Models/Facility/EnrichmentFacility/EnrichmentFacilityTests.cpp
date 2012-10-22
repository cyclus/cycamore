// EnrichmentFacilityTests.cpp
#include <gtest/gtest.h>

#include "EnrichmentFacilityTests.h"

#include "FacilityModelTests.h"
#include "ModelTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::SetUp() 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacilityTest::TearDown() 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Model* EnrichmentFacilityModelConstructor()
{
  return dynamic_cast<Model*>(new EnrichmentFacility());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
FacilityModel* EnrichmentFacilityConstructor()
{
  return dynamic_cast<FacilityModel*>(new EnrichmentFacility());
}

// //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// INSTANTIATE_TEST_CASE_P(EnrichmentFac, FacilityModelTests, Values(&EnrichmentFacilityConstructor));
// INSTANTIATE_TEST_CASE_P(EnrichmentFac, ModelTests, Values(&EnrichmentFacilityModelConstructor));

