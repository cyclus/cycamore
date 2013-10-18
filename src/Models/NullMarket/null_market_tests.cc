// null_market_tests.cc
#include <gtest/gtest.h>

#include "null_market.h"
#include "error.h"
#include "message.h"
#include "market_model_tests.h"
#include "generic_resource.h"
#include "model_tests.h"
#include "test_context.h"

#include <string>
#include <queue>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeNullMarket : public cycamore::NullMarket {
 protected:
  cyclus::Message::Ptr msg_;

 public:
  FakeNullMarket(cyclus::Context* ctx)
      : cycamore::NullMarket(ctx),
      cyclus::Model(ctx) {
    using std::string;
    string kg = "kg";
    string qual = "qual";
    cyclus::GenericResource::Ptr res = cyclus::GenericResource::CreateUntracked(1, kg, qual);
    cyclus::Transaction trans(this, cyclus::OFFER);
    msg_ = cyclus::Message::Ptr(new cyclus::Message(this, this, trans));
    msg_->trans().SetResource(res);
  }

  virtual ~FakeNullMarket() { }

  cyclus::Message::Ptr getMessage() {
    return msg_;
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* NullMarketModelConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::Model*>(new FakeNullMarket(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::MarketModel* NullMarketConstructor(cyclus::Context* ctx) {
  return dynamic_cast<cyclus::MarketModel*>(new FakeNullMarket(ctx));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class NullMarketTest : public ::testing::Test {
 protected:
  FakeNullMarket* src_market;
  cyclus::TestContext tc_;

  virtual void SetUp() {
    src_market = new FakeNullMarket(tc_.get());
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
  EXPECT_NO_THROW(src_market->ReceiveMessage(src_market->getMessage()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(NullMarket, MarketModelTests,
                        Values(&NullMarketConstructor));
INSTANTIATE_TEST_CASE_P(NullMarket, ModelTests,
                        Values(&NullMarketModelConstructor));

