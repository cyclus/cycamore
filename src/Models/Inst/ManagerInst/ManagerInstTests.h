// ManagerInstTests.h
#include <gtest/gtest.h>

#include "ManagerInst.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
class ManagerInstTests : public ::testing::Test 
{
 protected:
  ManagerInst* src_inst;
  
 public:  
  virtual void SetUp();
  virtual void TearDown();
};
