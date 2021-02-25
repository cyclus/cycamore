#ifndef STORAGE_TESTS_H_
#define STORAGE_TESTS_H_

#include <gtest/gtest.h>

#include "storage.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace cycamore {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StorageTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  Storage* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpStorage();
  void TestInitState(cycamore::Storage* fac);
  void TestAddMat(cycamore::Storage* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(cycamore::Storage* fac, double inv, double 

      proc, double ready, double stocks);
  void TestStocks(cycamore::Storage* fac, cyclus::CompMap v);
  void TestReadyTime(cycamore::Storage* fac, int t);
  void TestCurrentCap(cycamore::Storage* fac, double inv);

  std::vector<std::string> in_c1, out_c1;
  std::string in_r1;

  int residence_time;
  double throughput, max_inv_size;
  bool discrete_handling;
};
} // namespace cycamore
#endif // STORAGE_TESTS_H_

