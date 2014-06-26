#ifndef CYCAMORE_SRC_BATCH_REACTOR_TESTS_H_
#define CYCAMORE_SRC_BATCH_REACTOR_TESTS_H_

#include <gtest/gtest.h>

#include <map>
#include <string>

#include "batch_reactor.h"
#include "test_context.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class BatchReactorTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  BatchReactor* src_facility;

  // init params
  std::string in_c1, in_c2, out_c1, out_c2;
  std::string in_r1, in_r2, out_r1, out_r2;
  cyclus::toolkit::CommodityRecipeContext crctx;

  int n_batches, n_load, n_reserves;
  int process_time, refuel_time, preorder_time;
  double batch_size;

  std::string commodity;
  double capacity, cost;

  // init conds
  std::string rsrv_c, rsrv_r, core_c, core_r, stor_c, stor_r;
  int rsrv_n, core_n, stor_n;
  BatchReactor::InitCond ics;

  // changes changes
  int change_time;
  double frompref1, topref1, frompref2, topref2;
  std::map<std::string, double> commod_prefs;
  std::map<int, std::vector< std::pair< std::string, double > > > pref_changes;
  std::map<int, std::vector< std::pair< std::string, std::string > > >
      recipe_changes;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSource();

  /// @brief tests the initial state of a facility
  void TestInitState(BatchReactor* fac);

  /// @brief tests the number of batches in each buffer
  void TestBuffs(int nreserves, int ncore, int nstorage);

  /// @brief tests the BatchReactor's reserves_, by calling AddBatches_(mat),
  /// and confirming that there are n items and the last item has quantity qty
  void TestReserveBatches(cyclus::Material::Ptr mat, std::string commod,
                          int n, double qty);

  /// @brief calls MoveBatchIn_ and tests that the number of objects in core_ is
  /// n_core and the number of objects in reserves_ is n_reserves
  void TestBatchIn(int n_core, int n_reserves);

  /// @brief calls MoveBatchOut_ and tests that the number of objects in core_ is
  /// n_core and the number of objects in storage_ is n_storage
  void TestBatchOut(int n_core, int n_storage);
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_BATCH_REACTOR_TESTS_H_
