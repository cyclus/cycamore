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
  int n_batches, n_load, n_reserves;
  int process_time, refuel_time, preorder_time;
  int ic_reserves, ic_core, ic_storage;
  double batch_size;
  std::string in_commod, in_recipe, out_commod, out_recipe;
  std::string commodity;
  double capacity, cost;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSourceFacility();

  /// @brief sets src_facility's initial conditions
  void SetICs(BatchReactor::InitCond ics) { src_facility->ics_ = ics; }

  /// @brief tests the number of batches in each buffer
  void TestBuffs(int nreserves, int ncore, int nstorage);
  
  /// @brief tests the BatchReactor's reserves_, by calling AddBatches_(mat),
  /// and confirming that there are n items and the last item has quantity qty
  void TestReserveBatches(cyclus::Material::Ptr mat, int n, double qty);

  /// @brief calls MoveBatchIn_ and tests that the number of objects in core_ is
  /// n_core and the number of objects in reserves_ is n_reserves
  void TestBatchIn(int n_core, int n_reserves);
      
  /// @brief calls MoveBatchOut_ and tests that the number of objects in core_ is
  /// n_core and the number of objects in storage_ is n_storage
  void TestBatchOut(int n_core, int n_storage);
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_TESTS_
