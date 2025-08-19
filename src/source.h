#ifndef CYCAMORE_SRC_SOURCE_H_
#define CYCAMORE_SRC_SOURCE_H_

#include <set>
#include <vector>

#include "cyclus.h"
#include "cycamore_version.h"

// clang-format off
#pragma cyclus exec from cyclus.system import CY_LARGE_DOUBLE, CY_LARGE_INT, \
    CY_NEAR_ZERO
// clang-format on

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
               public cyclus::toolkit::CommodityProducer,
               public cyclus::toolkit::Position {
  friend class SourceTest;

 public:
  Source(cyclus::Context* ctx);
  virtual ~Source();

  virtual std::string version() { return CYCAMORE_VERSION; }

  // clang-format off
  #pragma cyclus note { \
    "doc": \
      "This facility acts as a source of material with a fixed throughput " \
      "(per time step) capacity and a lifetime capacity defined by a total " \
      "inventory size.  It offers its material as a single commodity. If a " \
      "composition recipe is specified, it provides that single material " \
      "composition to requesters.  If unspecified, the source provides " \
      "materials with the exact requested compositions.  The inventory size " \
      "and throughput both default to infinite.  Supplies material results " \
      "in corresponding decrease in inventory, and when the inventory size " \
      "reaches zero, the source can provide no more material.", \
  }

  #pragma cyclus def clone
  #pragma cyclus def schema
  #pragma cyclus def annotations
  #pragma cyclus def infiletodb
  #pragma cyclus def snapshot
  #pragma cyclus def snapshotinv
  #pragma cyclus def initinv
  // clang-format on

  virtual void InitFrom(Source* m);
  virtual void InitFrom(cyclus::QueryableBackend* b);

  virtual void Tick() {};
  virtual void Tock() {};

  virtual std::string str();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  virtual void EnterNotify();

  /// --- Facility Members ---
  /// perform module-specific tasks when entering the simulation
  virtual void Build(cyclus::Agent* parent);

  virtual void GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material>>& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr>>& responses);
 private:
  // clang-format off
  #pragma cyclus var { \
    "tooltip": "source output commodity", \
    "doc": "Output commodity on which the source offers material.", \
    "uilabel": "Output Commodity", \
    "uitype": "outcommodity" \
  }
  std::string outcommod;

  #pragma cyclus var { \
    "tooltip": "name of material recipe to provide", \
    "doc": "Name of composition recipe that this source provides regardless " \
           "of requested composition. If empty, source provides whatever " \
           "compositions are requested.", \
    "uilabel": "Output Recipe", \
    "default": "", \
    "uitype": "outrecipe" \
  }
  std::string outrecipe;

  #pragma cyclus var { \
    "doc": "Total amount of material this source has remaining. Every trade " \
           "decreases this value by the supplied material quantity. When it " \
           "reaches zero, the source cannot provide any more material.", \
    "default": CY_LARGE_DOUBLE, \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "uilabel": "Initial Inventory", \
    "units": "kg" \
  }
  double inventory_size;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "per time step throughput", \
    "units": "kg/(time step)", \
    "uilabel": "Maximum Throughput", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "Amount of commodity that can be supplied at each time step." \
  }
  double throughput;

  #pragma cyclus var { \
    "default": "unpackaged", \
    "tooltip": "name of package to provide material in", \
    "doc": "Name of package that this source provides. Offers will only be " \
           "made in packagable quantities.", \
    "uilabel": "Output Package Type", \
    "uitype": "package" \
  }
  std::string package;

  #pragma cyclus var { \
    "default": "unrestricted", \
    "tooltip": "name of transport unit to ship packages in", \
    "doc": "Name of transport unit that this source uses to ship packages of " \
           "material. Offers will only be made in shippable quantities. " \
           "Optional if packaging is used.", \
    "uilabel": "Output Transport Unit Type", \
    "uitype": "transportunit" \
  }
  std::string transport_unit;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double longitude;

  #pragma cyclus var { \
    "tooltip": "Material buffer" \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> inventory;
  // clang-format on

  cyclus::toolkit::Position coordinates;

  void RecordPosition();
  void SetPackage();
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SOURCE_H_
