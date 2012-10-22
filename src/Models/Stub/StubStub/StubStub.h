// StubStub.h
#ifndef _STUBSTUB_H
#define _STUBSTUB_H

#include "StubModel.h"

#include "Logger.h"
#include "QueryEngine.h"

/**
   The StubStub class inherits from the StubModel class and is 
   dynamically loaded by the Model class when requested. 
    
   This model will do nothing. This StubStub is intended as a skeleton 
   to guide the implementation of new StubStub models. 
 */
class StubStub : public StubModel {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */
 public:
  /**
     Default constructor for StubStub Class 
   */
  StubStub();

  /**
     every model should be destructable 
   */
  virtual ~StubStub();
    
  /**
     Initializes data members from data contained in a QueryEngine object
      
     @param qe is a QueryEngine object that contains initialization data 
   */
  virtual void initModuleMembers(QueryEngine* qe);
  
  /**
     every model needs a method to copy one object to another 
      
     @param src is the StubModel to copy 
   */
  virtual void cloneModuleMembersFrom(StubModel* src) ;

  /**
     every model should be able to print a verbose description 
   */
   virtual std::string str();

/* ------------------- */ 


/* --------------------
 * all STUBMODEL classes have these members
 * --------------------
 */

/* ------------------- */ 

};


#endif
