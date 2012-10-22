// StubMarket.cpp
// Implements the StubMarket class

#include "StubMarket.h"

#include "Logger.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubMarket::StubMarket() {};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StubMarket::~StubMarket() {};
    
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubMarket::initModuleMembers(QueryEngine* qe) { 
  QueryEngine* input = qe->queryElement("input");
};
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubMarket::cloneModuleMembers(MarketModel* src) { 
  StubMarket* src_stub = dynamic_cast<StubMarket*>(src);
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubMarket::str() { 
  return MarketModel::str(); 
};

/* -------------------- */


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubMarket::receiveMessage(msg_ptr msg) {};

/* -------------------- */


/* --------------------
 * all MARKETMODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubMarket::resolve() {};

/* -------------------- */


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructStubMarket() {
  return new StubMarket();
}

extern "C" void destructStubMarket(Model* model) {
  delete model;
}
/* -------------------- */

