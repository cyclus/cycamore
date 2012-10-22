// StubConverter.h
#ifndef _STUBCONVERTER_H
#define _STUBCONVERTER_H

#include <queue>

#include "ConverterModel.h"

#include "Logger.h"
#include "QueryEngine.h"

/**
   @class StubConverter 
    
   This ConverterModel is intended 
   as a skeleton to guide the implementation of new ConverterModel 
   models.  
   The StubConverter class inherits from the ConverterModel class and is 
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

class StubConverter : public ConverterModel {
/* --------------------
 * all MODEL classes have these public members
 * --------------------
 */
 public:
  /**
     Default constructor for StubConverter Class 
   */
  StubConverter() {};

  /**
     every model should be destructable 
   */
  virtual ~StubConverter() {};

  /**
     every model needs a method to initialize from XML 
      
     @param qe a pointer to a QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(QueryEngine* qe);

  /**
     every model needs a method to copy one object to another 
      
     @param src is the Converter to copy 
   */
  virtual void cloneModuleMembersFrom(ConverterModel* src) ;

  /**
     A verbose printer for the StubConverter
   */
  virtual std::string str();

/* ------------------- */ 
  

/* --------------------
 * all CONVERTERMODEL classes have these members
 * --------------------
 */  
 public:
  /**
     The convert function specific to the StubConverter 
     Converts between amounts of two commodities 
      
     @param convMsg is a message concerning one convertible commodity to 
     convert @param refMsg is a message concerning the commodity to 
     convert it to  
     @return the converted message 
   */
  virtual msg_ptr convert(msg_ptr convMsg, msg_ptr refMsg); 

/* ------------------- */ 


/* --------------------
 * _THIS_ CONVERTERMODEL class has these members
 * --------------------
 */

/* ------------------- */ 

};

#endif
