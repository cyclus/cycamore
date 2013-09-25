// Stubstub_inst.cc
// Implements the StubStubInst class


#include "stub_inst.h"

#include "logger.h"

namespace cycamore {

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubInst::StubInst(cyclus::Context* ctx) : cyclus::InstModel(ctx) {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubInst::~StubInst() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubInst::str() {
  return cyclus::InstModel::str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubInst::InitModuleMembers(cyclus::QueryEngine* qe) {}

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
extern "C" cyclus::Model* ConstructStubInst(cyclus::Context* ctx) {
  return new StubInst(ctx);
}
/* ------------------- */

} // namespace cycamore 
