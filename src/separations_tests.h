#ifndef SEPARATIONMATRIX_TESTS_H_
#define SEPARATIONMATRIX_TESTS_H_

#include <gtest/gtest.h>

#include "separations.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace separations {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationsTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  Separations* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSeparations();
  void TestInitState(separations::Separations* fac);
  void TestRequest(separations::Separations* fac, double cap);
  void TestAddMat(separations::Separations* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(separations::Separations* fac, double inv, double 
      proc, double rawbuff);

  std::string in_c1, out_c1, out_c2, out_c3;
  std::vector< std::string > outs;

  int process_time;
  double capacity, max_inv_size, cost;
};
} // namespace separations
#endif // SEPARATIONMATRIX_TESTS_H_

