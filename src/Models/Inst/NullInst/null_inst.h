// null_inst.h
#ifndef _NULLINST_H
#define _NULLINST_H

#include "InstModel.h"

/**
   @class NullInst 
    
   @section introduction Introduction 
   The NullInst is an institution type in Cyclus which performs the
   most basic instutituion functionality as defined by the InstModel
   class.
   
   @section detailedBehavior Detailed Behavior 
   The NullInst starts operation at the beginning of the 
   simulation and ends operation at the end of the simulation. It 
   populates its list of facilities when it is initialized and 
   determines its region as all Institution models do. When it 
   receives a message, it transmits that message immediately up to 
   its region or down to the appropriate facility without making 
   any changes. 
 */

class NullInst : public cyclus::InstModel {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */
 public:
  /**
     Default constructor
   */
  NullInst();
    
  /**
     Default destructor
   */
  virtual ~NullInst();

/* ------------------- */ 

};

#endif
