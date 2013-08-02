// StubStubInst.cpp
// Implements the StubStubInst class


#include "StubInst.h"

#include "Logger.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubInst::StubInst() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubInst::~StubInst() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubInst::str() { 
  return cyclus::InstModel::str(); 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::initModuleMembers(cyclus::QueryEngine* qe) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::enterSimulation(cyclus::Model* parent) {
  cyclus::InstModel::enterSimulation(parent);
}

/* ------------------- */ 


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::receiveMessage(cyclus::msg_ptr msg) {};

/* ------------------- */ 


/* --------------------
 * all INSTMODEL classes have these members
 * --------------------
 */

/* ------------------- */ 


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructStubInst() {
  return new StubInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructStubInst(cyclus::Model* model) {
  delete model;
}

/* ------------------- */ 



