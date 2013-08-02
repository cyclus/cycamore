// StubRegion.cpp
// Implements the StubRegion class

#include "StubRegion.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubRegion::StubRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubRegion::~StubRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubRegion::str() { 
  return cyclus::RegionModel::str(); 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubRegion::initModuleMembers(cyclus::QueryEngine* qe) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubRegion::enterSimulation(cyclus::Model* parent) {
  cyclus::RegionModel::enterSimulation(parent);
}

/* ------------------- */ 


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubRegion::receiveMessage(cyclus::msg_ptr msg) {}

/* ------------------- */ 


/* --------------------
 * all REGIONMODEL classes have these members
 * --------------------
 */

/* ------------------- */ 


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructStubRegion() {
      return new StubRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructStubRegion(cyclus::Model* model) {
      delete model;
}

/* ------------------- */ 
