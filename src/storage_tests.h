#ifndef STORAGE_TESTS_H_
#define STORAGE_TESTS_H_

#include <gtest/gtest.h>

#include "storage.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace storage {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StorageTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  Storage* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpStorage();
  void TestInitState(storage::Storage* fac);
  void TestRequest(storage::Storage* fac, double cap);
  void TestAddMat(storage::Storage* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(storage::Storage* fac, double inv, double 
      proc, double stocks);
  void TestStocks(storage::Storage* fac, cyclus::CompMap v);

  std::string in_c1, out_c1;
  std::string in_r1;

  int process_time;
  double capacity, max_inv_size, cost;
};
} // namespace storage
#endif // STORAGE_TESTS_H_

