// StubStub.cpp
// Implements the StubStub class

#include "StubStub.h"

#include "Logger.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubStub::StubStub() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubStub::~StubStub() {};
    
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStub::initModuleMembers(QueryEngine* qe) { 
  QueryEngine* input = qe->queryElement("input");
  //retrieve input data members here. For example :  
  //string query = "incommodity";
  //incommodity_ = lexical_cast<double>(input->getElementContent(query)); 
};
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStub::cloneModuleMembersFrom(StubModel* src) { 
  StubStub* src_stub = dynamic_cast<StubStub*>(src);
  //copy data members here. For example : 
  //incommodity_ = src_stub->inCommodity();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubStub::str() { 
  return StubModel::str(); 
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructStubStub() {
  return new StubStub();
}
extern "C" void destructStubStub(Model* model) { 
  delete model;
}

/* -------------------- */
