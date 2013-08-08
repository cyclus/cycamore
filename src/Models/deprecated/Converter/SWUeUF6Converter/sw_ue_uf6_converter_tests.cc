// sw_ue_uf6_converter_tests.cc
#include <gtest/gtest.h>

#include "sw_ue_uf6_converter.h"
#include "error.h"
#include "message.h"
#include "converter_model_tests.h"
#include "model_tests.h"
#include "test_inst.h"

#include <string>
#include <queue>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeSWUeUF6Converter : public SWUeUF6Converter {
  public:
    FakeSWUeUF6Converter() : SWUeUF6Converter() {
    }

    virtual ~FakeSWUeUF6Converter() {
    }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SWUeUF6ConverterModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new FakeSWUeUF6Converter());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::ConverterModel* SWUeUF6ConverterConstructor(){
  return dynamic_cast<cyclus::ConverterModel*>(new FakeSWUeUF6Converter());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class SWUeUF6ConverterTest : public ::testing::Test {
  protected:
    FakeSWUeUF6Converter* src_facility;
    FakeSWUeUF6Converter* new_facility;

    virtual void SetUp(){
      src_facility = new FakeSWUeUF6Converter();
      src_facility->SetParent(new TestInst());
      new_facility = new FakeSWUeUF6Converter();
      // for facilities that trade commodities, create appropriate markets here
    };

    virtual void TearDown() {
      delete src_facility;
      delete new_facility;
      // for facilities that trade commodities, delete appropriate markets here
    }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SWUeUF6ConverterTest, InitialState) {
  // Test things about the initial state of the facility here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SWUeUF6ConverterTest, CopyFreshModel) {
  new_facility->copyFreshModel(dynamic_cast<cyclus::Model*>(src_facility)); // deep copy
  EXPECT_NO_THROW(dynamic_cast<SWUeUF6Converter*>(new_facility)); // still a stub facility
  EXPECT_NO_THROW(dynamic_cast<FakeSWUeUF6Converter*>(new_facility)); // still a fake stub facility
  // Test that SWUeUF6Converter specific parameters are initialized in the deep copy method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SWUeUF6ConverterTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
  // Test SWUeUF6Converter specific aspects of the print method here
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(SWUeUF6ConverterTest, ReceiveMessage) {
  cyclus::Message::Ptr msg;
  // Test SWUeUF6Converter specific behaviors of the ReceiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
INSTANTIATE_TEST_CASE_P(SWUeUF6Conv, ConverterModelTests, Values(&SWUeUF6ConverterConstructor));
INSTANTIATE_TEST_CASE_P(SWUeUF6Conv, ModelTests, Values(&SWUeUF6ConverterModelConstructor));

