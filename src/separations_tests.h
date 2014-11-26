#ifndef CYCAMORE_SRC_SEPARATIONS_TESTS_H_
#define CYCAMORE_SRC_SEPARATIONS_TESTS_H_

#include <gtest/gtest.h>

#include "separations.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace cycamore {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationsTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  Separations* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSeparations();
  void TestInitState(cycamore::Separations* fac);
  void TestRequest(cycamore::Separations* fac, double cap);
  void TestAddMat(cycamore::Separations* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(cycamore::Separations* fac, double inv, double 
      proc, double rawbuff);

  std::string in_c1, out_c1, out_c2, out_c3;
  std::vector<std::string> ins, outs;

  int process_time;
  double capacity, max_inv_size, cost;
};
} // namespace cycamore
#endif // CYCAMORE_SRC_SEPARATIONS_TESTS_H_

