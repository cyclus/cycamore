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
};
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubStubComm::cloneModuleMembers(StubCommModel* src) { 
  StubStubComm* src_stub = <StubStubComm*>(src);
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
