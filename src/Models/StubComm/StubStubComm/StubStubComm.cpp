// StubStubCommComm.cpp
// Implements the StubStubCommComm class

#include "StubStubComm.h"

#include "Logger.h"
#include "QueryEngine.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubStubComm::StubStubComm() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubStubComm::~StubStubComm() {};
    
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStubComm::initModuleMembers(QueryEngine* qe) { 
  QueryEngine* input = qe->queryElement("input");
  //retrieve input data members here. For example :  
  //string query = "incommodity";
  //incommodity_ = lexical_cast<double>(input->getElementContent(query));
};
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStubComm::cloneModuleMembersFrom(StubCommModel* src) { 
  StubStubComm* src_stub = dynamic_cast<StubStubComm*>(src);
  //copy data members here. For example : 
  //incommodity_ = src_stub->inCommodity();
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubStubComm::str() { 
  return StubCommModel::str(); 
};

/* -------------------- */


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStubComm::receiveMessage(msg_ptr msg) {};

/* -------------------- */


/* --------------------
 * all STUBCOMMMODEL classes have these members
 * --------------------
 */

/* -------------------- */


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructStubStubComm() {
  return new StubStubComm();
}
extern "C" void destructStubStubComm(Model* model) {
  delete model;
}

/* -------------------- */
