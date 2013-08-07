// manager_inst.h
#ifndef _MANAGERINST_H
#define _MANAGERINST_H

#include "InstModel.h"
#include "Builder.h"
#include "CommodityProducerManager.h"
#include "CommodityProducer.h"

/**
   @class ManagerInst 
   
   @section introduction Introduction 
   
   @section detailedBehavior Detailed Behavior 
*/
class ManagerInst : public cyclus::InstModel, 
  public cyclus::SupplyDemand::CommodityProducerManager,
  public cyclus::ActionBuilding::Builder
{
 public:
  /**
     Default constructor
   */
  ManagerInst();
    
  /**
     Default destructor
   */
  virtual ~ManagerInst();

  /**
     perform any actions required after prototype has been added to
     the list of available prototypes
     @param prototype the prototype to register
   */
  virtual void registerAvailablePrototype(cyclus::Prototype* prototype);
  
  /**
     perform any registration functionality after a clone has been 
     built
     @param clone the built (cloned) prototype
   */
  virtual void registerCloneAsBuilt(cyclus::Prototype* clone);

  /**
     perform any registration functionality before a clone is 
     decommissioned(deleted)
     @param clone the to-be-decommissioned prototype
   */
  virtual void registerCloneAsDecommissioned(cyclus::Prototype* clone);

  /**
     write information about a commodity producer to a stream
     @param producer the producer
   */
  void writeProducerInformation(cyclus::SupplyDemand::CommodityProducer* producer);
};

#endif
