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
    msg_ptr msg_;
  public:
    FakeNullMarket() : NullMarket() {
      string kg = "kg";
      string qual = "qual";
      gen_rsrc_ptr res = gen_rsrc_ptr(new GenericResource(kg, qual, 1));
      Transaction trans(this, OFFER);
      msg_ = msg_ptr(new Message(this, this, trans));
      msg_->trans().setResource(res);
    }

    virtual ~FakeNullMarket() {
    }

    msg_ptr getMessage(){return msg_;}
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Model* NullMarketModelConstructor(){
  return dynamic_cast<Model*>(new FakeNullMarket());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MarketModel* NullMarketConstructor(){
  return dynamic_cast<MarketModel*>(new FakeNullMarket());
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
INSTANTIATE_TEST_CASE_P(NullMarket, MarketModelTests, Values(&NullMarketConstructor));
INSTANTIATE_TEST_CASE_P(NullMarket, ModelTests, Values(&NullMarketModelConstructor));

