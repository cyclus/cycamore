// StubStubComm.h
#ifndef _STUBSTUBCOMM_H
#define _STUBSTUBCOMM_H

#include "StubCommModel.h"

#include "Logger.h"

/**
   The StubStubComm class inherits from the StubModel class and is 
   dynamically loaded by the Model class when requested. 
    
   This model will do nothing. This StubCommModel is intended as a 
   skeleton to guide the implementation of new StubComm models. 
 */
class StubStubComm : public StubCommModel {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */
 public:
  /**
     Default constructor for StubStub Class 
   */
  StubStubComm();
  
  /**
     every model should be destructable 
   */
  virtual ~StubStubComm();
    
  /**
     Initializes the data members of this module based on the data in the QueryEngine
      
     @param qe the QueryEngine object containing initialization data 
   */
  virtual void initModuleMembers(QueryEngine* qe);
  
  /**
     Copies the data members of another StubCommModel 

     @param src is the StubCommModel to copy 
   */
  virtual void cloneModuleMembersFrom(StubCommModel* src) ;
  
  /**
     every model should be able to print a verbose description 
   */
  virtual std::string str();

/* ------------------- */ 


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */
 public:
  /**
     The StubStubComm should ignore incoming messages 
   */
  virtual void receiveMessage(msg_ptr msg);
  
/* -------------------- */


/* --------------------
 * all STUBCOMMMODEL classes have these members
 * --------------------
 */

/* ------------------- */ 

};


#endif
