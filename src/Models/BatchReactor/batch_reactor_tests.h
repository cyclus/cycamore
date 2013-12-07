#ifndef CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_TESTS_
#define CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_TESTS_

#include <gtest/gtest.h>

#include <string>

#include "test_context.h"

#include "batch_reactor.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BatchReactorTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  BatchReactor* src_facility;
  int n_batch, n_load, n_reserve;
  int process, refuel, preorder;
  double batch_size;
  std::string in_commod, in_recipe, out_commod, out_recipe;
  std::string commodity;
  double capacity, cost;

  virtual void SetUp();
  virtual void TearDown();
  /// void InitSrcFacility();
  /// void InitWorld();
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_TESTS_
