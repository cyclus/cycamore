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

/// This facility acts as a simple conversion facility from its input commodity
/// to its output commodity. It has a fixed throughput (per time step), and 
/// converts without regard to the composition of the input material.
class Conversion
  : public cyclus::Facility,
    public cyclus::toolkit::Position  {
 public:
  Conversion(cyclus::Context* ctx);

  virtual ~Conversion();

  virtual std::string version() { return CYCAMORE_VERSION; }

  // clanag-format off
  #pragma cyclus note { \
    "doc": \
    " A conversion facility that accepts materials and products and with a \n"\
    " fixed throughput (per time step) converts them into its outcommod. " \
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
    "doc": "throughput the facility can convert at each time step" \
  }
  double throughput;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "input buffer capacity", \
    "uilabel": "Maximum Storage of Input Buffer", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "Total capacity of the input buffer. The amount requested per " \
    "time step is the minimum the conversion throughput and this capacity " \
    "minus the amount of material in the input buffer." \
  }
  double input_capacity;

  /// this facility holds a certain amount of material
  #pragma cyclus var {'capacity': 'input_capacity'}
  cyclus::toolkit::ResBuf<cyclus::Material> input;

  /// a buffer for outgoing material
  cyclus::toolkit::ResBuf<cyclus::Material> output;
  // clang-format on

};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_CONVERSION_H_

