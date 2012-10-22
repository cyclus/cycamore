// StubConverter.cpp
// Implements the StubConverter class

#include "Logger.h"
#include "CycException.h"
#include "InputXML.h"
#include "StubConverter.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubConverter::initModuleMembers(QueryEngine* qe) { 
  QueryEngine* input = qe->queryElement("input"); 
  //retrieve input data members here. For example :  
  //string query = "incommodity";
  //incommodity_ = lexical_cast<double>(input->getElementContent(query));
};
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubConverter::cloneModuleMembersFrom(ConverterModel* src) { 
  StubConverter* src_conv = dynamic_cast<StubConverter*>(src);
  //copy data members here. For example : 
  //incommodity_ = src_stub->inCommodity();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubConverter::str() { 
  return ConverterModel::str(); 
};

/* ------------------- */ 


/* --------------------
 * all CONVERTERMODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
msg_ptr StubConverter::convert(msg_ptr convMsg, msg_ptr refMsg) {
  throw CycException("The StubConverter should not be used to convert things.");
}

/* ------------------- */ 

    
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructStubConverter() {
  return new StubConverter();
}
extern "C" void destructStubConverter(Model* model) {
  delete model;
}

/* ------------------- */ 


