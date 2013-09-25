// manager_inst.h
#ifndef _MANAGERINST_H
#define _MANAGERINST_H

#include "inst_model.h"
#include "builder.h"
#include "commodity_producer_manager.h"
#include "commodity_producer.h"

namespace cycamore {

/**
   @class ManagerInst

   @section introduction Introduction

   @section detailedBehavior Detailed Behavior
*/
class ManagerInst : public cyclus::InstModel,
  public cyclus::supply_demand::CommodityProducerManager,
  public cyclus::action_building::Builder {
 public:
  /**
     Default constructor
   */
  ManagerInst(cyclus::Context* ctx);

  /**
     Default destructor
   */
  virtual ~ManagerInst();

  virtual cyclus::Model* Clone() {
    ManagerInst* m = new ManagerInst(context());
    m->initfrom(this);
    return m;
  }

  /**
     perform any actions required after prototype has been added to
     the list of available prototypes
     @param prototype the prototype to register
   */
  virtual void RegisterAvailablePrototype(std::string prototype);

  /**
     perform any registration functionality after a clone has been
     built
     @param clone the built (cloned) prototype
   */
  virtual void RegisterCloneAsBuilt(cyclus::Model* clone);

  /**
     perform any registration functionality before a clone is
     decommissioned(deleted)
     @param clone the to-be-decommissioned prototype
   */
  virtual void RegisterCloneAsDecommissioned(cyclus::Model* clone);

  /**
     write information about a commodity producer to a stream
     @param producer the producer
   */
  void WriteProducerInformation(cyclus::supply_demand::CommodityProducer*
                                producer);
};
} // namespace cycamore 
#endif
