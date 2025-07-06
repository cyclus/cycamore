#ifndef CYCAMORE_SRC_SINK_H_
#define CYCAMORE_SRC_SINK_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cyclus.h"
#include "cycamore_version.h"

// clang-format off
#pragma cyclus exec from cyclus.system import CY_LARGE_DOUBLE, CY_LARGE_INT, \
    CY_NEAR_ZERO
// clang-format on

namespace cycamore {

class Context;

/// This facility acts as a sink of materials and products with a fixed
/// throughput (per time step) capacity and a lifetime capacity defined by a
/// total inventory size.  The inventory size and throughput capacity both
/// default to infinite. If a recipe is provided, it will request material with
/// that recipe. Requests are made for any number of specified commodities.
class Sink : public cyclus::Facility,
             public cyclus::toolkit::Position {
              
 public:
  Sink(cyclus::Context* ctx);
  virtual ~Sink();

  virtual std::string version() { return CYCAMORE_VERSION; }

    // clang-format off
  #pragma cyclus note { \
    "doc": \
      " A sink facility that accepts materials and products with a fixed\n" \
      " throughput (per time step) capacity and a lifetime capacity defined by\n" \
      " a total inventory size. The inventory size and throughput capacity\n" \
      " both default to infinite. If a recipe is provided, it will request\n" \
      " material with that recipe. Requests are made for any number of\n" \
      " specified commodities.\n", \
  }

  #pragma cyclus decl
  // clang-format on

  virtual std::string str();
  virtual void EnterNotify();
  virtual void Tick();
  virtual void Tock();

  /// @brief SinkFacilities request Materials of their given commodity.  Note
  /// that it is assumed the Sink operates on a single resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  /// @brief SinkFacilities request Products of their given commodity.  Note
  /// that it is assumed the Sink operates on a single resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
      GetGenRsrcRequests();

  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptMatlTrades(
      const std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                                  cyclus::Material::Ptr>>& responses);

  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptGenRsrcTrades(
      const std::vector<std::pair<cyclus::Trade<cyclus::Product>,
                                  cyclus::Product::Ptr>>& responses);

  /// @brief SinkFacilities update request amount using random behavior
  virtual void SetRequestAmt();

  /// @brief SinkFacilities update request time using random behavior
  virtual void SetNextBuyTime();

  /// add a commodity to the set of input commodities
  inline void AddCommodity(std::string name) {
    in_commods.push_back(name);
  }

  /// sets the size of the storage inventory for received material
  inline void SetMaxInventorySize(double size) {
    max_inv_size = size;
    inventory.capacity(size);
  }

  /// @return the maximum inventory storage size
  inline double MaxInventorySize() const {
    return inventory.capacity();
  }

  /// @return the current inventory storage size
  inline double InventorySize() const {
    return inventory.quantity();
  }

  /// determines the amount to request
  inline double SpaceAvailable() const {
    return std::min(capacity, std::max(0.0, inventory.space()));
  }

  /// sets the capacity of a material generated at any given time step
  inline void Capacity(double cap) { capacity = cap; }

  /// @return the reception capacity at any given time step
  inline double Capacity() const { return capacity; }

  /// @return the input commodities
  inline const std::vector<std::string>& input_commodities() const {
    return in_commods;
  }

  /// @return the input commodity preferences
  inline const std::vector<double>& input_commodity_preferences() const {
    return in_commod_prefs;
  }

  private:
  double requestAmt;
  int nextBuyTime;

  // clang-format off
  #pragma cyclus var { \
    "tooltip": "input commodities", \
    "doc": "commodities that the sink facility accepts", \
    "uilabel": "List of Input Commodities", \
    "uitype": ["oneormore", "incommodity"] \
  }
  std::vector<std::string> in_commods;

  #pragma cyclus var { \
    "default": [], \
    "doc": "preferences for each of the given commodities, in the same order. " \
           "Defaults to 1 if unspecified", \
    "uilabel": "In Commodity Preferences", \
    "range": [None, [CY_NEAR_ZERO, CY_LARGE_DOUBLE]], \
    "uitype": ["oneormore", "range"] \
  }
  std::vector<double> in_commod_prefs;

  #pragma cyclus var { \
    "default": "", \
    "tooltip": "requested composition", \
    "doc": "name of recipe to use for material requests, where the default " \
           "(empty string) is to accept everything", \
    "uilabel": "Input Recipe", \
    "uitype": "inrecipe" \
  }
  std::string recipe_name;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "sink maximum inventory size", \
    "uilabel": "Maximum Inventory", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "total maximum inventory size of sink facility" \
  }
  double max_inv_size;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "sink capacity", \
    "uilabel": "Maximum Throughput", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "capacity the sink facility can accept at each time step" \
  }
  double capacity;

  #pragma cyclus var { \
    "capacity": "max_inv_size" \
  }
  cyclus::toolkit::ResBuf<cyclus::Resource> inventory;

  #pragma cyclus var { \
    "default": "None", \
    "tooltip": "type of random behavior when setting the size of the request", \
    "uitype": "combobox", \
    "uilabel": "Random Size", \
    "categorical": ["None", "UniformReal", "UniformInt", "NormalReal", \
                    "NormalInt"], \
    "doc": "type of random behavior to use. Default None; other options are " \
           "'UniformReal', 'UniformInt', 'NormalReal', and 'NormalInt'" \
  }
  std::string random_size_type;

  #pragma cyclus var { \
    "default": 1.0, \
    "tooltip": "fraction of available space to determine the mean", \
    "uilabel": "Random Size Mean", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "When a normal distribution is used to determine the size of the "\
           "request, this is the fraction of available space to use as the "\
           "mean. Default 1.0; note that values significantly above 1 without "\
           "a correspondingly large std dev may be inefficient." \
  }
  double random_size_mean;

  #pragma cyclus var { \
    "default": 0.1, \
    "tooltip": "fraction of available space to determine the std dev", \
    "uilabel": "Random Size Std Dev", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "When a normal distribution is used to determine the size of the "\
           "request, this is the fraction of available space to use as the "\
           "standard deviation. Default 0.1" \
  }
  double random_size_stddev;

  #pragma cyclus var { \
    "default": "None", \
    "tooltip": "type of random behavior when setting the timing of the "\
               "request", \
    "uitype": "combobox", \
    "uilabel": "Random Timing", \
    "categorical": ["None", "UniformInt", "NormalInt"], \
    "doc": "type of random behavior to use. Default None; other options are "\
           "'UniformInt' and 'NormalInt'. When using 'UniformInt', also "\
           "set 'random_frequency_min' and 'random_frequency_max'. For "\
           "'NormalInt', set 'random_frequency_mean' and "\
           "'random_frequency_stddev'; min and max are optional." \
  }
  std::string random_frequency_type;

  #pragma cyclus var { \
    "default": 1, \
    "tooltip": "mean of the random frequency", \
    "uilabel": "Random Frequency Mean", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "When a normal distribution is used to determine the frequency of "\
           "the request, this is the mean. Default 1" \
  }
  double random_frequency_mean;

  #pragma cyclus var { \
    "default": 1, \
    "tooltip": "std dev of the random frequency", \
    "uilabel": "Random Frequency Std Dev", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "doc": "When a normal distribution is used to determine the frequency of "\
           "the request, this is the standard deviation. Default 1" \
  }
  double random_frequency_stddev;

  #pragma cyclus var { \
    "default": 1, \
    "tooltip": "lower bound of the random frequency", \
    "uilabel": "Random Frequency Lower Bound", \
    "uitype": "range", \
    "range": [1, CY_LARGE_INT], \
    "doc": "When a random distribution is used to determine the frequency of "\
           "the request, this is the lower bound. Default 1" \
  }
  int random_frequency_min;

  #pragma cyclus var { \
    "default": CY_LARGE_INT, \
    "tooltip": "upper bound of the random frequency", \
    "uilabel": "Random Frequency Upper Bound", \
    "uitype": "range", \
    "range": [1, CY_LARGE_INT], \
    "doc": "When a random distribution is used to determine the frequency of "\
           "the request, this is the upper bound. Default CY_LARGE_INT" \
  }
  int random_frequency_max;

  #pragma cyclus var { \
    "default": True, \
    "tooltip": "Whether to persist packaging in the sink.", \
    "doc": "Boolean value about whether to keep packaging. If true, "\
           "packaging will not be stripped upon acceptance into the sink. "\
           "If false, package type will be stripped immediately upon "\
           "acceptance. Has no effect if the incoming material is not "\
           "packaged.", \
    "uilabel": "Keep Packaging", \
    "uitype": "bool" \
  }
  bool keep_packaging;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value should "\
           "be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value should "\
           "be expressed in degrees as a double." \
  }
  double longitude;
  // clang-format on

  cyclus::toolkit::Position coordinates;

  void RecordPosition();
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SINK_H_
