// stub_inst.h
#if !defined(_STUBINST_H)
#define _STUBINST_H

#include "InstModel.h"
#include "QueryEngine.h"

/**
   @class StubInst 
    
   This InstModel is intended 
   as a skeleton to guide the implementation of new InstModel models. 
    
   The StubInst class inherits from the InstModel class and is 
   dynamically loaded by the Model class when requested. 
    
   @section intro Introduction 
   Place an introduction to the model here. 
    
   @section modelparams Model Parameters 
   Place a description of the required input parameters which define the 
   model implementation. 
    
   @section optionalparams Optional Parameters 
   Place a description of the optional input parameters to define the 
   model implementation. 
    
   @section detailed Detailed Behavior 
   Place a description of the detailed behavior of the model. Consider 
   describing the behavior at the tick and tock as well as the behavior 
   upon sending and receiving materials and messages. 
 */

class StubInst : public cyclus::InstModel {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */
 public:
  /**
     Default constructor for StubInst Class 
   */
  StubInst();

  /**
     every model should be destructable 
   */
  virtual ~StubInst();

  /**
     every model should be able to print a verbose description 
   */
   virtual std::string str();
  
  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(cyclus::QueryEngine* qe);

  /**
     perform all necessary actions for the model to enter the simulation
   */
  virtual void enterSimulation(cyclus::Model* parent);

/* ------------------- */ 

   
/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */
 public:
   /**
      The StubInst should ignore incoming messages 
    */
   virtual void receiveMessage(cyclus::msg_ptr msg);
   
/* -------------------- */


/* --------------------
 * all INSTMODEL classes have these members
 * --------------------
 */

/* ------------------- */ 


/* --------------------
 * This INSTMODEL class has these members
 * --------------------
 */

/* ------------------- */ 

};

#endif
