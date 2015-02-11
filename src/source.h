#ifndef CYCAMORE_SRC_SOURCE_H_
#define CYCAMORE_SRC_SOURCE_H_

#include <set>
#include <vector>

#include "cyclus.h"

namespace cycamore {

class Context;

/// This facility acts as a source of material with a fixed throughput (per
/// time step) capacity and a lifetime capacity defined by a total inventory
/// size.  It offers its material as a single commodity. If a composition
/// recipe is specified, it provides that single material composition to
/// requesters.  If unspecified, the source provides materials with the exact
/// requested compositions.  The inventory size and throughput both default to
/// infinite.  Supplies material results in corresponding decrease in
/// inventory, and when the inventory size reaches zero, the source can provide
/// no more material.
class Source : public cyclus::Facility,
  public cyclus::toolkit::CommodityProducer {
  friend class SourceTest;
 public:

  Source(cyclus::Context* ctx);

  virtual ~Source();

  #pragma cyclus note { \
    "doc": "This facility acts as a source of material with a fixed throughput (per\n" \
           " time step) capacity and a lifetime capacity defined by a total inventory\n" \
           " size.  It offers its material as a single commodity. If a composition\n" \
           " recipe is specified, it provides that single material composition to\n" \
           " requesters.  If unspecified, the source provides materials with the exact\n" \
           " requested compositions.  The inventory size and throughput both default to\n" \
           " infinite.  Supplies material results in corresponding decrease in\n" \
           " inventory, and when the inventory size reaches zero, the source can provide\n" \
           " no more material.\n" \
  }

  #pragma cyclus def clone
  #pragma cyclus def schema
  #pragma cyclus def annotations
  #pragma cyclus def infiletodb
  #pragma cyclus def snapshot
  #pragma cyclus def snapshotinv
  #pragma cyclus def initinv

  virtual void InitFrom(Source* m);

  virtual void InitFrom(cyclus::QueryableBackend* b);

  virtual void EnterNotify();

  virtual void Tick() {};

  virtual void Tock() {};

  virtual std::string str();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

 private:
  #pragma cyclus var { \
    "tooltip": "source output commodity", \
    "doc": "Output commodity on which the source offers material.", \
    "uitype": "outcommodity", \
  }
  std::string outcommod;

  #pragma cyclus var { \
    "tooltip": "name of material recipe to provide", \
    "doc": "Name of composition recipe that this source provides regardless of requested composition." \
           " If empty, source creates and provides whatever compositions are requested.", \
    "default": "", \
    "uitype": "recipe", \
  }
  std::string outrecipe;

  #pragma cyclus var { \
    "default": 1e299, \
    "tooltip": "per time step throughput", \
    "units": "kg/(time step)", \
    "doc": "amount of commodity that can be supplied at each time step", \
  }
  double throughput;

  #pragma cyclus var { \
    "doc": "Total amount of material this source has remaining." \
           " Every trade decreases this value by the supplied material quantity'." \
           " When it reaches zero, the source cannot provide any more material.", \
    "default": 1e299, \
    "units": "kg", \
  }
  double inventory_size;

};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SOURCE_H_
