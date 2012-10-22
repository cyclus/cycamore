// StubFacility.cpp
// Implements the StubFacility class

#include "StubFacility.h"

#include "Logger.h"
#include "CycException.h"

using namespace std;

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubFacility::StubFacility() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubFacility::~StubFacility() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void StubFacility::initModuleMembers(QueryEngine* qe) {
  QueryEngine* input = qe->queryElement("input");
  //retrieve input data members here. For example :  
  //string query = "incommodity";
  //incommodity_ = lexical_cast<double>(input->getElementContent(query));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void StubFacility::cloneModuleMembersFrom(FacilityModel* src) {
  StubFacility* src_stub = dynamic_cast<StubFacility*>(src);
  //copy data members here. For example : 
  //incommodity_ = src_stub->inCommodity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
std::string StubFacility::str() {
  return FacilityModel::str();
};

/* ------------------- */ 


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubFacility::receiveMessage(msg_ptr msg) {}

/* ------------------- */ 


/* --------------------
 * all FACILITYMODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
vector<rsrc_ptr> StubFacility::removeResource(Transaction order) {}
    
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubFacility::addResource(Transaction trans, std::vector<rsrc_ptr> manifest){}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubFacility::handleTick(int time){}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubFacility::handleTock(int time){}

/* ------------------- */ 


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

extern "C" Model* constructStubFacility() {
  return new StubFacility();
}

extern "C" void destructStbuFacility(Model* model) {
  delete model;
}

/* ------------------- */ 

