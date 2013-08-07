// stub_converter_tests.cc
#include <gtest/gtest.h>

#include "stub_converter.h"
#include "error.h"
#include "Message.h"
#include "ConverterModelTests.h"
#include "ModelTests.h"
#include "TestInst.h"

#include <string>
#include <queue>

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class FakeStubConverter : public StubConverter {
  public:
    FakeStubConverter() : StubConverter() {
    }

    virtual ~FakeStubConverter() {
    }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* StubConverterModelConstructor(){
  return dynamic_cast<cyclus::Model*>(new FakeStubConverter());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::ConverterModel* StubConverterConstructor(){
  return dynamic_cast<cyclus::ConverterModel*>(new FakeStubConverter());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class StubConverterTest : public ::testing::Test {
  protected:
    FakeStubConverter* src_facility;
    FakeStubConverter* new_facility; 

    virtual void SetUp(){
      src_facility = new FakeStubConverter();
      src_facility->setParent(new TestInst());
      new_facility = new FakeStubConverter();
      // for facilities that trade commodities, create appropriate markets here
    };

    virtual void TearDown() {
      delete src_facility;
      delete new_facility;
      // for facilities that trade commodities, delete appropriate markets here
    }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubConverterTest, InitialState) {
  // Test things about the initial state of the facility here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubConverterTest, CopyFreshModel) {
  new_facility->copyFreshModel(dynamic_cast<cyclus::Model*>(src_facility)); // deep copy
  EXPECT_NO_THROW(dynamic_cast<StubConverter*>(new_facility)); // still a stub facility
  EXPECT_NO_THROW(dynamic_cast<FakeStubConverter*>(new_facility)); // still a fake stub facility
  // Test that StubConverter specific parameters are initialized in the deep copy method here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubConverterTest, Print) {
  EXPECT_NO_THROW(std::string s = src_facility->str());
  // Test StubConverter specific aspects of the print method here
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
TEST_F(StubConverterTest, ReceiveMessage) {
  cyclus::msg_ptr msg;
  // Test StubConverter specific behaviors of the receiveMessage function here
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
INSTANTIATE_TEST_CASE_P(StubConv, ConverterModelTests, Values(&StubConverterConstructor));
INSTANTIATE_TEST_CASE_P(StubConv, ModelTests, Values(&StubConverterModelConstructor));

