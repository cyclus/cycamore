#ifndef CYCLUS_STORAGES_STORAGE_H_
#define CYCLUS_STORAGES_STORAGE_H_

#include <list>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"
#include "cycamore_version.h"
#include "cyclus.h"

// clang-format off
#pragma cyclus exec from cyclus.system import CY_LARGE_DOUBLE, CY_LARGE_INT, \
    CY_NEAR_ZERO
// clang-format on

namespace cycamore {
/// @class Storage
///
/// This Facility is intended to hold materials for a user specified
/// amount of time in order to model a storage facility with a certain
/// residence time or holdup time.
/// The Storage class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// This Agent was initially developed to support the fco code-to-code
/// comparison. It's very similar to the "NullFacility" of years
/// past. Its purpose is to hold materials and release them only
/// after some period of delay time.
///
/// @section agentparams Agent Parameters
/// in_commods is a vector of strings naming the commodities that this facility
/// receives
/// out_commods is a string naming the commodity that in_commod is stocks into
/// residence_time is the minimum number of timesteps between receiving and
/// offering
/// in_recipe (optional) describes the incoming resource by recipe
///
/// @section optionalparams Optional Parameters
/// max_inv_size is the maximum capacity of the inventory storage
/// throughput is the maximum processing capacity per timestep
/// package is the name of the package type to ship
///
/// @section detailed Detailed Behavior
///
/// Tick:
///   Nothing really happens on the tick.
///
/// Tock:
///   On the tock, any material that has been waiting for long enough (delay
///   time) is placed in the stocks buffer.
///
///   Any brand new inventory that was received in this timestep is placed into
///   the processing queue to begin waiting.
///
/// Making Requests:
///   This facility requests all of the in_commod that it can.
///
/// Receiving Resources:
///   Anything of the in_commod that is received by this facility goes into the
///   inventory.
///
/// Making Offers:
///   Any stocks material in the stocks buffer is offered to the market.
///
/// Sending Resources:
///   Matched resources are sent immediately.
class Storage : public cyclus::Facility,
                public cyclus::toolkit::CommodityProducer,
                public cyclus::toolkit::Position {
 public:
  /// @param ctx the cyclus context for access to simulation-wide parameters
  Storage(cyclus::Context* ctx);

#pragma cyclus decl

  // clang-format off
  #pragma cyclus note { \
    "doc": "Storage is a simple facility which accepts any number of "\
            "commodities and holds them for a user specified amount of "\
            "time. The commodities accepted are chosen based on the "\
            "specified preferences list. Once the desired amount of "\
            "material has entered the facility it is passed into a "\
            "'processing' buffer where it is held until the residence "\
            "time has passed. The material is then passed into a 'ready' "\
            "buffer where it is queued for removal. Currently, all input "\
            "commodities are lumped into a single output commodity. "\
            "Storage also has the functionality to handle materials in "\
            "discrete or continuous batches. Discrete mode, which is the "\
            "default, does not split or combine material batches. "\
            "Continuous mode, however, divides material batches if "\
            "necessary in order to push materials through the facility "\
            "as quickly as possible." \
  }
  // clang-format on

  /// A verbose printer for the Storage Facility
  virtual std::string str();

  // --- Facility Members ---

  // --- Agent Members ---
  /// Sets up the Storage Facility's trade requests
  virtual void EnterNotify();

  /// The handleTick function specific to the Storage.
  virtual void Tick();

  /// The handleTick function specific to the Storage.
  virtual void Tock();

  virtual std::string version() { return CYCAMORE_VERSION; }

 protected:
  /// @brief adds a material into the incoming commodity inventory
  /// @param mat the material to add to the incoming inventory.
  /// @throws if there is trouble with pushing to the inventory buffer.
  void AddMat_(cyclus::Material::Ptr mat);

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_();

  /// @brief Move as many ready resources as allowable into stocks
  /// @param cap current throughput capacity
  void ProcessMat_(double cap);

  /// @brief move ready resources from processing to ready at a certain time
  /// @param time the time of interest
  void ReadyMatl_(int time);

  // --- Storage Members ---

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() { return (inventory_tracker.space()); }

  /// @brief returns total capacity
  inline double capacity() { return inventory_tracker.capacity(); }

  /// @brief returns the time key for ready materials
  inline int ready_time() { return context()->time() - residence_time; }

  // clang-format off
  #pragma cyclus var { \
    "tooltip": "input commodity", \
    "doc": "commodities accepted by this facility", \
    "uilabel": "Input Commodities", \
    "uitype": ["oneormore", "incommodity"] \
  }
  std::vector<std::string> in_commods;

  #pragma cyclus var { \
    "default": [], \
    "doc": "preferences for each of the given commodities, in the same order. "\
           "Defaults to 1 if unspecified", \
    "uilabel": "In Commodity Preferences", \
    "range": [None, [CY_NEAR_ZERO, CY_LARGE_DOUBLE]], \
    "uitype": ["oneormore", "range"] \
  }
  std::vector<double> in_commod_prefs;

  #pragma cyclus var { \
    "tooltip": "output commodity", \
    "doc": "commodity produced by this facility. Multiple commodity tracking "\
           "is currently not supported, one output commodity catches all "\
           "input commodities.", \
    "uilabel": "Output Commodities", \
    "uitype": ["oneormore", "outcommodity"] \
  }
  std::vector<std::string> out_commods;

  #pragma cyclus var { \
    "default": "", \
    "tooltip": "input recipe", \
    "doc": "recipe accepted by this facility, if unspecified a dummy recipe "\
           "is used", \
    "uilabel": "Input Recipe", \
    "uitype": "inrecipe" \
  }
  std::string in_recipe;

  #pragma cyclus var { \
    "default": 0, \
    "tooltip": "residence time (timesteps)", \
    "doc": "the minimum holding time for a received commodity "\
           "(timesteps)", \
    "units": "time steps", \
    "uilabel": "Residence Time", \
    "uitype": "range", \
    "range": [0, 12000] \
  }
  int residence_time;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "throughput per timestep (kg)", \
    "doc": "the max amount that can be moved through the facility "\
           "per timestep (kg)", \
    "uilabel": "Throughput", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "units": "kg" \
  }
  double throughput;

  #pragma cyclus var { \
    "default": CY_LARGE_DOUBLE, \
    "tooltip": "maximum inventory size (kg)", \
    "doc": "the maximum amount of material that can be in all storage "\
           "buffer stages", \
    "uilabel": "Maximum Inventory Size", \
    "uitype": "range", \
    "range": [0.0, CY_LARGE_DOUBLE], \
    "units": "kg" \
  }
  double max_inv_size;

  #pragma cyclus var { \
    "default": False, \
    "tooltip": "Bool to determine how Storage handles batches", \
    "doc": "Determines if Storage will divide resource objects. Only "\
           "controls material handling within this facility. If true, "\
           "batches are handled as discrete quanta. Otherwise, batches "\
           "may be divided during processing. Default false (continuous).", \
    "uilabel": "Batch Handling" \
  }
  bool discrete_handling;

  #pragma cyclus var { \
    "default": "unpackaged", \
    "tooltip": "Output package", \
    "doc": "Outgoing material will be packaged when trading.", \
    "uilabel": "Package", \
    "uitype": "package" \
  }
  std::string package;

  #pragma cyclus var { \
    "default": "unrestricted", \
    "tooltip": "Output transport unit", \
    "doc": "Outgoing material, after packaging, will be restricted by "\
           "transport unit when trading.", \
    "uilabel": "Transport Unit", \
    "uitype": "transportunit" \
  }
  std::string transport_unit;

  #pragma cyclus var { \
    "tooltip": "Incoming material buffer" \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> inventory;

  #pragma cyclus var { \
    "tooltip": "Output material buffer" \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> stocks;

  #pragma cyclus var { \
    "tooltip": "Buffer for material held for required residence_time" \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> ready;

  // list of input times for materials entering the processing buffer
  #pragma cyclus var { \
    "default": [], \
    "internal": True \
  }
  std::list<int> entry_times;

  #pragma cyclus var { \
    "tooltip": "Buffer for material still waiting for required residence_time" \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> processing;

  #pragma cyclus var { \
    "tooltip": "Total Inventory Tracker to restrict maximum agent inventory" \
  }
  cyclus::toolkit::TotalInvTracker inventory_tracker;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value "\
           "should be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value "\
           "should be expressed in degrees as a double." \
  }
  double longitude;
  // clang-format on

  cyclus::toolkit::Position coordinates;

  void RecordPosition();

  friend class StorageTest;

 private:
// Code Injection
#include "toolkit/matl_buy_policy.cycpp.h"
#include "toolkit/matl_sell_policy.cycpp.h"
};

}  // namespace cycamore

#endif  // CYCLUS_STORAGES_STORAGE_H_
