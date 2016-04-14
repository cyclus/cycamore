#ifndef MIXINGFAB_TESTS_H_
#define MIXINGFAB_TESTS_H_

#include <gtest/gtest.h>

#include "mixing_fab.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace cycamore {
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  class MixingFabTest : public ::testing::Test {
  protected:
    cyclus::TestContext tc_;
    MixingFab* mf_facility_;
    
    virtual void SetUp();
    virtual void TearDown();
    void InitParameters();
    
    void SetUpMixingFab();
    void SetInputInv(MixingFab* fac, std::vector<Material::Ptr> mat);
    
    void TestInitState(MixingFab* fac);
    void TestBuffers(MixingFab* fac,
                                    std::vector<double> in_inv,
                                    double out_inv);
    void TestOutputComposition(MixingFab* fac, cyclus::CompMap v);
    
    std::vector<std::string> in_com;
    std::vector<double> in_frac;
    std::vector<double> in_cap;
    
    
    std::string out_com;
    double out_cap;
    
    double throughput;
  };
} // namespace cycamore
#endif // MIXINGFAB_TESTS_H_

