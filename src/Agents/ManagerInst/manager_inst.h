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
class ManagerInst : public cyclus::Institution,
  public cyclus::CommodityProducerManager,
  public cyclus::Builder {
 public:
  /**
     Default constructor
   */
  ManagerInst(cyclus::Context* ctx);

  /**
     Default destructor
   */
  virtual ~ManagerInst();

  virtual cyclus::Agent* Clone() {
    ManagerInst* m = new ManagerInst(context());
    m->InitFrom(this);
    return m;
  }

  void InitFrom(ManagerInst* m) {
    Institution::InitFrom(m);
    commod_producers_ = m->commod_producers_;
  }

  virtual void InitFrom(cyclus::QueryEngine* qe);

  /**
     perform any actions required after prototype has been added to
     the list of available prototypes
     @param prototype the prototype to register
   */
  void RegisterAvailablePrototype(std::string prototype);

  /**
     perform any registration functionality after a clone has been
     built
     @param clone the built (cloned) prototype
   */
  virtual void BuildNotify(cyclus::Agent* clone);

  /**
     perform any registration functionality before a clone is
     decommissioned(deleted)
     @param clone the to-be-decommissioned prototype
   */
  virtual void DecomNotify(cyclus::Agent* clone);

  /**
     write information about a commodity producer to a stream
     @param producer the producer
   */
  void WriteProducerInformation(cyclus::CommodityProducer*
                                producer);
};
} // namespace cycamore 
#endif
