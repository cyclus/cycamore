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
    

  public:
    typedef cyclus::toolkit::ResBuf<cyclus::Material> InvBuffer;
    cyclus::TestContext tc_;
    MixingFab* mf_facility_;
    
    virtual void SetUp();
    virtual void TearDown();
    
    std::vector<std::string> in_com;
    std::vector<double> in_frac;
    std::vector<double> in_cap;
    
    
    std::string out_com;
    double out_cap;
    
    double throughput;
    
    void SetThroughput(double thpt) { throughput = thpt; mf_facility_->throughput = thpt;}
    
    void SetStream_comds(std::vector<std::string> comds)   { in_com  = comds;  mf_facility_->in_commods   = comds; }
    void SetStream_ratio(std::vector<double> ratio)        { in_frac = ratio;  mf_facility_->mixing_ratio = ratio; }
    void SetStream_capacity(std::vector<double> cap)       { in_cap  = cap;    mf_facility_->in_buf_size  = cap; }

    void SetOutStream_comds(std::string com)  { out_com = com; mf_facility_->out_commod      = com; }
    void SetOutStream_capacity(double cap)    { out_cap = cap; mf_facility_->output_buf_size = cap; }
    void SetInputInv(std::vector<Material::Ptr> mat);

    
    std::vector<std::string>  GetStream_comds() { return mf_facility_->in_commods; }
    std::vector<double> GetStream_ratio()       { return mf_facility_->mixing_ratio; }
    std::vector<double> GetStream_capacity()    { return mf_facility_->in_buf_size; }
    
    std::string GetOutStream_comds()  { return mf_facility_->out_commod; }
    double GetOutStream_capacity()    { return mf_facility_->output_buf_size; }
    double GetThroughput()    { return mf_facility_->throughput; }
    
    
    InvBuffer* GetOutPutBuffer() { return &mf_facility_->output;}
    std::map<std::string, InvBuffer > GetStreamBuffer() { return mf_facility_->streambufs;}
    
    

    
  };
} // namespace cycamore
#endif // MIXINGFAB_TESTS_H_

