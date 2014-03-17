// inpro_reactor_tests.cc
#include <gtest/gtest.h>

#include "inpro_reactor.h"

#include "test_context.h"
#include <string>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class InproReactorTest : public ::testing::Test {
  
 protected:
  cyclus::TestContext tc_;
  cycamore::InproReactor* src_facility;
  int lencycle, nbatch;
  double in_loadcore, out_loadcore;
  std::string in_commod, in_recipe, out_commod, out_recipe;
  std::string commodity;
  double capacity, cost;

  virtual void SetUp();
  virtual void TearDown();
  void InitSrcFacility();
};
