// NullMarketTests.cpp
#include <gtest/gtest.h>

#include "NullMarket.h"
#include "CycException.h"
#include "Message.h"
#include "MarketModelTests.h"
#include "GenericResource.h"
#include "ModelTests.h"

#include <string>
#include <queue>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeNullMarket : public NullMarket {
  protected:
    cyclus::msg_ptr msg_;
  public:
    FakeNullMarket() : NullMarket() {
      string kg = "kg";
      string qual = "qual";
      gen_rsrc_ptr res = gen_rsrc_ptr(new cyclus::GenericResource(kg, qual, 1));
      cyclus::Transaction trans(this, cyclus::OFFER);
      msg_ = cyclus::msg_ptr(new cyclus::Message(this, this, trans));
      msg_->trans().setResource(res);
    }

    virtual ~FakeNullMarket() {
    }

    cyclus::msg_ptr getMessage(){return msg_;}
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* NullMarketModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new FakeNullMarket());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::MarketModel* NullMarketConstructor(){
  return dynamic_cast<cyclus::MarketModel*>(new FakeNullMarket());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullMarketTest : public ::testing::Test {
  protected:
    FakeNullMarket* src_market;

    virtual void SetUp(){
      src_market = new FakeNullMarket();
    };

    virtual void TearDown() {
      delete src_market;
    }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(NullMarketTest, Print) {
  EXPECT_NO_THROW(std::string s = src_market->str());
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(NullMarketTest, ReceiveMessage) {
  EXPECT_NO_THROW(src_market->receiveMessage(src_market->getMessage()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(NullMarket, cyclus::MarketModelTests, Values(&NullMarketConstructor));
INSTANTIATE_TEST_CASE_P(NullMarket, ModelTests, Values(&NullMarketModelConstructor));

