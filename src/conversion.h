#ifndef CYCAMORE_SRC_CONVERSION_H_
#define CYCAMORE_SRC_CONVERSION_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cyclus.h"
#include "cycamore_version.h"

// clang-format off
#pragma cyclus exec from cyclus.system import CY_LARGE_DOUBLE, CY_LARGE_INT, CY_NEAR_ZERO
// clang-format on

namespace cycamore {

class Context;

/// Black-box uranium conversion between commodities, preserving uranium mass
/// and isotopic composition. All material quantities represent kgU; chemical
/// species and their non-uranium mass are not modeled.
class Conversion
  : public cyclus::Facility {
  friend class ConversionTest;
 public:
  Conversion(cyclus::Context* ctx);

  virtual ~Conversion();

  virtual std::string version() { return CYCAMORE_VERSION; }

  // clang-format off
  #pragma cyclus note { \
    "doc": \
    "Accepts Materials and converts between uranium commodities without " \
    "modeling chemistry or changing uranium mass or isotopic composition. " \
    "All quantities are kgU, not compound mass. Input requests fill available " \
    "feed storage independently of processing throughput. Processing occurs " \
    "before exchange, so newly received feed is processed next timestep. " \
    "Throughput limits processing only; accumulated product can ship in one " \
    "timestep and output storage is unlimited." \
    }

  #pragma cyclus decl
  // clang-format on

  virtual std::string str();

  virtual void EnterNotify();

  virtual void Tick();

  virtual void Tock();
  
  double AvailableFeedstockCapacity();

  void Convert();


  /// @brief Conversion Facilities request Materials of their given commodity.
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
  GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
      commod_requests);

  virtual void GetMatlTrades(
      const std::vector< cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Conversion Facilities place accepted trade Materials in their Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

 

 private:
  // Code Injection:
  #include "toolkit/position.cycpp.h"

  // clang-format off
  #pragma cyclus var { \
    "default": 1.0, \
    "uilabel": "Variable Cost Per Unit", \
    "doc": "Conversion cost per kgU, added to the carried value of feed " \
           "when bidding output material. Must be finite and non-negative.", \
    "units": "Unit of Currency/kgU", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "schema": '<optional><element name="unit_cost"><data type="double">' \
              '<param name="minInclusive">0</param>' \
              '<param name="maxInclusive">' + str(CY_LARGE_DOUBLE) + '</param>' \
              '</data></element></optional>', \
  }
  double unit_cost;

  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Unit Cost Modifiers", \
    "doc": "Request cost modifiers in input commodity order; defaults to " \
           "zero for each commodity. Lower values favor a commodity. " \
           "Arbitrary modifiers remove economic meaning from reported " \
           "arc_cost, but do not affect carried material value.", \
    "units": "Unit of Currency/kgU", \
    "uitype": ["oneormore", "range"], \
    "range": [None, [-CY_LARGE_DOUBLE, CY_LARGE_DOUBLE]], \
    "schema": '<optional><element name="unit_cost_mods"><oneOrMore>' \
              '<element name="val"><data type="double">' \
              '<param name="minInclusive">' + str(-CY_LARGE_DOUBLE) + '</param>' \
              '<param name="maxInclusive">' + str(CY_LARGE_DOUBLE) + '</param>' \
              '</data></element></oneOrMore></element></optional>', \
  }
  std::vector<double> unit_cost_mods;

  /// all facilities must have at least one input commodity
  #pragma cyclus var { \
    "tooltip": "input commodities", \
    "doc": "commodities that the conversion facility accepts", \
    "uilabel": "List of Input Commodities", \
    "uitype": ["oneormore", "incommodity"] \
  }
  std::vector<std::string> incommods;

  #pragma cyclus var { \
    "tooltip": "output commodity", \
    "doc": "Output commodity on which the conversion facility offers material.", \
    "uilabel": "Output Commodity", \
    "uitype": "outcommodity", \
  }
  std::string outcommod;

  /// Conversion throughput per timestep
  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "conversion throughput per timestep", \
    "uilabel": "Maximum conversion throughput", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "units": "kgU/timestep", \
    "doc": "Maximum uranium processing rate; does not limit feed purchases " \
           "or shipments of accumulated product. Zero disables processing.", \
    "schema": '<optional><element name="throughput"><data type="double">' \
              '<param name="minInclusive">0</param>' \
              '<param name="maxInclusive">' + str(CY_LARGE_DOUBLE) + '</param>' \
              '</data></element></optional>', \
  }
  double throughput;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "input buffer capacity", \
    "uilabel": "Maximum Storage of Input Buffer", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "units": "kgU", \
    "doc": "Total feed storage capacity in kgU. Requests fill the remaining " \
           "space regardless of processing throughput." \
  }
  double input_capacity;

  /// this facility holds a certain amount of material
  #pragma cyclus var {'capacity': 'input_capacity'}
  cyclus::toolkit::ResBuf<cyclus::Material> input;

  /// a buffer for outgoing material
  #pragma cyclus var {}
  cyclus::toolkit::ResBuf<cyclus::Material> output;
  // clang-format on

};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_CONVERSION_H_
