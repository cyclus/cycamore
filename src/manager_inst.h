// manager_inst.h
#ifndef CYCAMORE_SRC_MANAGER_INST_H_
#define CYCAMORE_SRC_MANAGER_INST_H_

#include "cyclus.h"

namespace cycamore {

/// @class ManagerInst
/// @section introduction Introduction
/// @section detailedBehavior Detailed Behavior
class ManagerInst : public cyclus::Institution,
  public cyclus::toolkit::CommodityProducerManager,
  public cyclus::toolkit::Builder {
 public:
  /// Default constructor
  ManagerInst(cyclus::Context* ctx);

  /// Default destructor
  virtual ~ManagerInst();

  #pragma cyclus

  /// perform any registration functionality after a clone has been
  /// built
  /// @param clone the built (cloned) prototype
  virtual void BuildNotify(cyclus::Agent* clone);

  /// perform any registration functionality before a clone is
  /// decommissioned(deleted)
  /// @param clone the to-be-decommissioned prototype
  virtual void DecomNotify(cyclus::Agent* clone);

  /// write information about a commodity producer to a stream
  /// @param producer the producer
  void WriteProducerInformation(cyclus::toolkit::CommodityProducer*
                                producer);
};
} // namespace cycamore 

#endif  // CYCAMORE_SRC_MANAGER_INST_H_
