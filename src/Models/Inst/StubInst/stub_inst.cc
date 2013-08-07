// Stubstub_inst.cc
// Implements the StubStubInst class


#include "stub_inst.h"

#include "logger.h"

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
void StubInst::InitModuleMembers(cyclus::QueryEngine* qe) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::EnterSimulation(cyclus::Model* parent) {
  cyclus::InstModel::EnterSimulation(parent);
}

/* ------------------- */


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::ReceiveMessage(cyclus::Message::Ptr msg) {};

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



