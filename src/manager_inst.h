#ifndef CYCAMORE_SRC_MANAGER_INST_H_
#define CYCAMORE_SRC_MANAGER_INST_H_

#include "cyclus.h"

namespace cycamore {

/// @class ManagerInst
/// @section introduction Introduction
/// @section detailedBehavior Detailed Behavior
/// @warning The ManagerInst is experimental
class ManagerInst
    : public cyclus::Institution,
      public cyclus::toolkit::CommodityProducerManager,
      public cyclus::toolkit::Builder {
 public:
  /// Default constructor
  ManagerInst(cyclus::Context* ctx);

  /// Default destructor
  virtual ~ManagerInst();

  #pragma cyclus

  #pragma cyclus note {"doc": "An institution that owns and operates a " \
                              "manually entered list of facilities in " \
                              "the input file"}

  /// enter the simulation and register any children present
  virtual void EnterNotify();

  /// register a new child
  virtual void BuildNotify(Agent* m);

  /// unregister a child
  virtual void DecomNotify(Agent* m);

  /// write information about a commodity producer to a stream
  /// @param producer the producer
  void WriteProducerInformation(cyclus::toolkit::CommodityProducer*
                                producer);

 private:
  /// register a child
  void Register_(cyclus::Agent* agent);

  /// unregister a child
  void Unregister_(cyclus::Agent* agent);

  #pragma cyclus var {"tooltip": "facility prototypes", \
                      "doc": "a facility to be managed by the institution", \
                      "uitype": "facility"}
  std::vector<std::string> prototypes;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_MANAGER_INST_H_
