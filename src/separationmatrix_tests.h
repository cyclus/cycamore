#ifndef SEPARATIONMATRIX_TESTS_H_
#define SEPARATIONMATRIX_TESTS_H_

#include <gtest/gtest.h>

#include "separationmatrix.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace separationmatrix {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SeparationMatrixTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  SeparationMatrix* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpSeparationMatrix();
  void TestInitState(separationmatrix::SeparationMatrix* fac);
  void TestRequest(separationmatrix::SeparationMatrix* fac, double cap);
  void TestAddMat(separationmatrix::SeparationMatrix* fac, 
      cyclus::Material::Ptr mat);
  void TestBuffers(separationmatrix::SeparationMatrix* fac, double inv, double 
      proc, double rawbuff);

  std::string in_c1, out_c1, out_c2, out_c3;
  std::vector< std::string > outs;

  int process_time;
  double capacity, max_inv_size, cost;
};
} // namespace separationmatrix
#endif // SEPARATIONMATRIX_TESTS_H_

