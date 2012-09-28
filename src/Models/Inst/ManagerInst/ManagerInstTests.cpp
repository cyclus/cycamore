#include "ManagerInstTests.h"
#include "InstModelTests.h"
#include "ModelTests.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void ManagerInstTests::SetUp()
{
  src_inst = new ManagerInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void ManagerInstTests::TearDown()
{
  delete src_inst;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Model* ManagerInstModelConstructor()
{
  return dynamic_cast<Model*>(new ManagerInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
InstModel* ManagerInstConstructor()
{
  return dynamic_cast<InstModel*>(new ManagerInst());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(ManagerInst, InstModelTests, Values(&ManagerInstConstructor));
INSTANTIATE_TEST_CASE_P(ManagerInst, ModelTests, Values(&ManagerInstModelConstructor));

