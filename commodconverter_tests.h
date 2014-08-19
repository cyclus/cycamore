#ifndef COMMODCONVERTER_TESTS_H_
#define COMMODCONVERTER_TESTS_H_

#include <gtest/gtest.h>

#include "commodconverter.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace commodconverter {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class CommodConverterTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  commodconverter::CommodConverter* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void TestInitState(commodconverter::CommodConverter* fac);

  std::string in_c1, out_c1;
  std::string in_r1, out_r1;

  int process_time;
  double capacity, max_inv_size;
};
} // namespace commodconverter
#endif // COMMODCONVERTER_TESTS_H_

