// batch_reactor_tests.cc
#include <gtest/gtest.h>

#include "batch_reactor.h"

#include "test_context.h"
#include <string>

class TestMarket;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BatchReactorTest : public ::testing::Test {
  
 protected:
  cyclus::TestContext tc_;
  cycamore::BatchReactor* src_facility;
  int lencycle, nbatch;
  double in_loadcore, out_loadcore;
  std::string in_commod, in_recipe, out_commod, out_recipe;
  TestMarket* incommod_market;
  TestMarket* outcommod_market;
  std::string commodity;
  double capacity, cost;

  virtual void SetUp();
  virtual void TearDown();
  void InitSrcFacility();
  void InitWorld();
};
