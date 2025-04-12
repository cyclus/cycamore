#ifndef CYCLUS_STORAGES_STORAGE_H_
#define CYCLUS_STORAGES_STORAGE_H_

#include <string>
#include <list>
#include <vector>

#include "cyclus.h"
#include "cycamore_version.h"

#include "boost/shared_ptr.hpp"

#pragma cyclus exec from cyclus.system import CY_LARGE_DOUBLE, CY_LARGE_INT, CY_NEAR_ZERO

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
/// comparison.
/// It's very similar to the "NullFacility" of years
/// past. Its purpose is to hold materials and release them only
/// after some period of delay time.
///
/// @section agentparams Agent Parameters
/// in_commods is a vector of strings naming the commodities that this facility receives
/// out_commods is a string naming the commodity that in_commod is stocks into
/// residence_time is the minimum number of timesteps between receiving and offering
/// in_recipe (optional) describes the incoming resource by recipe
///
/// @section optionalparams Optional Parameters
/// sell_quantity restricts selling to only integer multiples of this value
/// max_inv_size is the maximum capacity of the inventory storage
/// throughput is the maximum processing capacity per timestep
/// active_buying_frequency_type is the type of distribution used to determine the length of the active buying period
/// active_buying_val is the length of the active buying period if active_buying_frequency_type is Fixed
/// active_buying_min is the minimum length of the active buying period if active_buying_frequency_type is Uniform (required) or 
/// Normal (optional)
/// active_buying_max is the maximum length of the active buying period if active_buying_frequency_type is Uniform (required) or
/// Normal (optional)
/// active_buying_mean is the mean length of the active buying period if active_buying_frequency_type is Normal
/// active_buying_std is the standard deviation of the active buying period if active_buying_frequency_type is Normal
/// active_buying_end_probability is the probability that at any given timestep, the agent ends the active buying 
///                               period if the active buying frequency type is Binomial
/// active_buying_disruption_probability is the probability that in any given cycle, the agent undergoes a disruption 
///                               (disrupted active period) if the active buying frequency type is FixedWithDisruption
/// active_buying_disruption is the length of the disrupted active cycle if the active buying frequency type is 
///                               FixedWithDisruption
/// dormant_buying_frequency_type is the type of distribution used to determine the length of the dormant buying period
/// dormant_buying_val is the length of the dormant buying period if dormant_buying_frequency_type is Fixed
/// dormant_buying_min is the minimum length of the dormant buying period if dormant_buying_frequency_type is Uniform (required) or
/// Normal (optional)
/// dormant_buying_max is the maximum length of the dormant buying period if dormant_buying_frequency_type is Uniform (required) or
/// Normal (optional)
/// dormant_buying_mean is the mean length of the dormant buying period if dormant_buying_frequency_type is Normal
/// dormant_buying_std is the standard deviation of the dormant buying period if dormant_buying_frequency_type is Normal
/// dormant_buying_end_probability is the probability that at any given timestep, the agent ends the dormant buying period if
///                               the dormant buying frequency type is Binomial
/// dormant_buying_disruption_probability is the probability that in any given cycle, the agent undergoes a disruption (disrupted
///                               offline period) if the dormant buying frequency type is FixedWithDisruption
/// dormant_buying_disruption is the length of the disrupted dormant cycle if the dormant buying frequency type is 
///                               FixedWithDisruption
/// buying_size_type is the type of distribution used to determine the size of buy requests, as a fraction of the current capacity
/// buying_size_val is the size of the buy request for Fixed  buying_size_type
/// buying_size_min is the minimum size of the buy request if buying_size_type is Uniform (required) or Normal (optional)
/// buying_size_max is the maximum size of the buy request if buying_size_type is Uniform (required) or Normal (optional)
/// buying_size_mean is the mean size of the buy request if buying_size_type is Normal
/// buying_size_stddev is the standard deviation of the buy request if buying_size_type is Normal
/// package is the name of the package type to ship
///
/// @section detailed Detailed Behavior
///
/// Tick:
/// Nothing really happens on the tick.
///
/// Tock:
/// On the tock, any material that has been waiting for long enough (delay
/// time) is placed in the stocks buffer.
///
/// Any brand new inventory that was received in this timestep is placed into
/// the processing queue to begin waiting.
///
/// Making Requests:
/// This facility requests all of the in_commod that it can.
///
/// Receiving Resources:
/// Anything of the in_commod that is received by this facility goes into the
/// inventory.
///
/// Making Offers:
/// Any stocks material in the stocks buffer is offered to the market.
///
/// Sending Resources:
/// Matched resources are sent immediately.
class Storage
  : public cyclus::Facility,
    public cyclus::toolkit::CommodityProducer {
 public:  
  /// @param ctx the cyclus context for access to simulation-wide parameters
  Storage(cyclus::Context* ctx);

  #pragma cyclus decl

  #pragma cyclus note {"doc": "Storage is a simple facility which accepts any number of commodities " \
                              "and holds them for a user specified amount of time. The commodities accepted "\
                              "are chosen based on the specified preferences list. Once the desired amount of material "\
                              "has entered the facility it is passed into a 'processing' buffer where it is held until "\
                              "the residence time has passed. The material is then passed into a 'ready' buffer where it is "\
                              "queued for removal. Currently, all input commodities are lumped into a single output commodity. "\
                              "Storage also has the functionality to handle materials in discrete or continuous batches. Discrete "\
                              "mode, which is the default, does not split or combine material batches. Continuous mode, however, "\
                              "divides material batches if necessary in order to push materials through the facility as quickly "\
                              "as possible."}

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
  ///   @brief sets up the distributions for the buy policy
  void InitBuyPolicyParameters();
  ///   @brief adds a material into the incoming commodity inventory
  ///   @param mat the material to add to the incoming inventory.
  ///   @throws if there is trouble with pushing to the inventory buffer.
  void AddMat_(cyclus::Material::Ptr mat);

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_();

  /// @brief Move as many ready resources as allowable into stocks
  /// @param cap current throughput capacity
  void ProcessMat_(double cap);

  /// @brief move ready resources from processing to ready at a certain time
  /// @param time the time of interest
  void ReadyMatl_(int time);

    /* --- Storage Members --- */

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() {
    return (inventory_tracker.space()); }

  /// @brief returns total capacity
  inline double capacity() { return inventory_tracker.capacity(); }

  /// @brief returns the time key for ready materials
  int ready_time(){ return context()->time() - residence_time; }

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodities accepted by this facility",\
                      "uilabel":"Input Commodities",\
                      "uitype":["oneormore","incommodity"]}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"default": [],\
                      "doc":"preferences for each of the given commodities, in the same order."\
                      "Defauts to 1 if unspecified",\
                      "uilabel":"In Commody Preferences", \
                      "range": [None, [CY_NEAR_ZERO, CY_LARGE_DOUBLE]], \
                      "uitype":["oneormore", "range"]}
  std::vector<double> in_commod_prefs;

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility. Multiple commodity tracking is"\
                      " currently not supported, one output commodity catches all input commodities.",\
                      "uilabel":"Output Commodities",\
                      "uitype":["oneormore","outcommodity"]}
  std::vector<std::string> out_commods;

  #pragma cyclus var {"default":"",\
                      "tooltip":"input recipe",\
                      "doc":"recipe accepted by this facility, if unspecified a dummy recipe is used",\
                      "uilabel":"Input Recipe",\
                      "uitype":"inrecipe"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"residence time (timesteps)",\
                      "doc":"the minimum holding time for a received commodity (timesteps).",\
                      "units":"time steps",\
                      "uilabel":"Residence Time", \
                      "uitype": "range", \
                      "range": [0, 12000]}
  int residence_time;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"sell quantity (kg)",\
                      "doc":"material will be sold in integer multiples of this quantity. If"\
                      " the buffer contains less than the sell quantity, no material will be"\
                      " offered", \
                      "uilabel":"Sell Quantity",\
                      "uitype": "range", \
                      "range": [0.0, CY_LARGE_DOUBLE], \
                      "units": "kg"}
  double sell_quantity;

  #pragma cyclus var {"default": CY_LARGE_DOUBLE,\
                     "tooltip":"throughput per timestep (kg)",\
                     "doc":"the max amount that can be moved through the facility per timestep (kg)",\
                     "uilabel":"Throughput",\
                     "uitype": "range", \
                     "range": [0.0, CY_LARGE_DOUBLE], \
                     "units":"kg"}
  double throughput;

  #pragma cyclus var {"default": CY_LARGE_DOUBLE,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the maximum amount of material that can be in all storage buffer stages",\
                      "uilabel":"Maximum Inventory Size",\
                      "uitype": "range", \
                      "range": [0.0, CY_LARGE_DOUBLE], \
                      "units":"kg"}
  double max_inv_size;

  #pragma cyclus var {"default": False,\
                      "tooltip":"Bool to determine how Storage handles batches",\
                      "doc":"Determines if Storage will divide resource objects. Only controls material "\
                            "handling within this facility, has no effect on DRE material handling. "\
                            "If true, batches are handled as discrete quanta, neither split nor combined. "\
                            "Otherwise, batches may be divided during processing. Default to false (continuous))",\
                      "uilabel":"Batch Handling"}
  bool discrete_handling;

  #pragma cyclus var {"default": "Fixed",\
                      "tooltip": "Type of active buying frequency",\
                      "doc": "Options: Fixed, Uniform, Normal, Binomial, FixedWithDisruption. "\
                      "Fixed requires active_buying_val. Uniform "\
                      "requires active_buying_min and active_buying_max.  Normal "\
                      "requires active_buying_mean and active_buying_std, with optional "\
                      "active_buying_min and active_buying_max. Binomial requires active_buying_end_probability."\
                      "FixedWithDisruption has a probability that any given cycle will have a disrupted, "\
                      "active length.  Once per cycle, a Bernoulli distribution (Binomial dist "\
                      "with N=1) will be sampled to determine if typical or disrupted cycle. If typical, "\
                      "active_buying_val is cycle length. If disrupted, active_buying_disruption.",\
                      "uitype": "combobox",\
                      "categorical": ["Fixed", "Uniform", "Normal", "Binomial", "FixedWithDisruption"],\
                      "uilabel": "Active Buying Frequency Type"}
  std::string active_buying_frequency_type;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Fixed active buying frequency",\
                      "doc": "The length in time steps of the active buying period. Required for fixed "\
                      "active_buying_frequency_type. Must be greater than or equal to 1 (i.e., agent "\
                      "cannot always be dormant)",\
                      "uitype": "range", \
                      "range": [1, CY_LARGE_INT], \
                      "uilabel": "Active Buying Frequency Value"}
  int active_buying_val;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Active buying distribution minimum",\
                      "doc": "The minimum length in time steps of the active buying period. Required for "\
                      "Uniform and optional for Normal active_buying_frequency_type. Must be greater than "\
                      "or equal to 1 ",\
                      "uitype": "range", \
                      "range": [1, CY_LARGE_INT], \
                      "uilabel": "Active Buying Frequency Minimum"}
  int active_buying_min;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Active buying distribution maximum",\
                      "doc": "The maximum length in time steps of the active buying period. Required for "\
                      "Uniform active_buying_frequency_type, optional for Normal. Must be greater than or equal to active_buying_min ",\
                      "uitype": "range", \
                      "range": [1, CY_LARGE_INT], \
                      "uilabel": "Active Buying Frequency Maximum"}
  int active_buying_max;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Active buying distribution mean",\
                      "doc": "The mean length in time steps of the active buying period. Required for "\
                      "Normal active_buying_frequency_type. Must be greater than or equal to 1 ",\
                      "uitype": "range", \
                      "range": [1.0, CY_LARGE_DOUBLE], \
                      "uilabel": "Active Buying Frequency Mean"}
  double active_buying_mean;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Active buying distribution standard deviation",\
                      "doc": "The standard deviation of the length in time steps of the active buying period. "\
                      "Required for Normal active_buying_frequency_type. Must be greater than or equal to 0 ",\
                      "uitype": "range", \
                      "range": [0.0, CY_LARGE_DOUBLE], \
                      "uilabel": "Active Buying Frequency Standard Deviation"}
  double active_buying_stddev;

  #pragma cyclus var {"default": 0,\
                      "tooltip": "Probability that agent will go offline during the next time step",\
                      "doc": "Binomial distribution has a fixed probability of going dormant at any given "\
                      "timestep, like a weighted coin flip. Required for Binomial active_buying_frequency_type. "\
                      "Must be between 0 and 1",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Active Buying Offline Probability"}
  double active_buying_end_probability;

  #pragma cyclus var {"default": 0,\
                      "tooltip": "Probability that a cycle contains a disruption",\
                      "doc": "Probability that the agent undergoes a disruption (disrupted active period) "\
                      "during any given cycle. Required for FixedWithDisruption active_buying_frequency_type.",\
                      "uitype": "range",\
                      "range": [0.0, 1.0],\
                      "uilabel": "Active Buying Disruption Probability"}
  double active_buying_disruption_probability;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Fixed length of disrupted active cycle",\
                      "doc": "When a active cycle is disrupted, this is length of the active period instead "\
                      "of active_buying_val. Required for FixedWithDisruption active_buying_frequency_type",\
                      "uitype": "range",\
                      "range": [0, CY_LARGE_INT]}
  int active_buying_disruption;

  #pragma cyclus var {"default": "Fixed",\
                      "tooltip": "Type of dormant buying frequency",\
                      "doc": "Options: Fixed, Uniform, Normal, Binomial, FixedWithDisruption. "\
                      "Fixed requires dormant_buying_val. "\
                      "Uniform requires dormant_buying_min and dormant_buying_max. Normal requires "\
                      "dormant_buying_mean and dormant_buying_std, with optional dormant_buying_min "\
                      "and dormant_buying_max. Binomial requires dormant_buying_end_probability. "\
                      "FixedWithDisruption has a probability that any given cycle will have a disrupted, "\
                      "or long, outage.  Once per cycle, a Bernoulli distribution (Binomial dist "\
                      "with N=1) will be sampled to determine if typical or disrupted cycle. If typical, "\
                      "dormant_buying_val is cycle length. If disrupted, dormant_buying_disruption.",\
                      "uitype": "combobox",\
                      "categorical": ["Fixed", "Uniform", "Normal", "Binomial", "FixedWithDisruption"],\
                      "uilabel": "Dormant Buying Frequency Type"}
  std::string dormant_buying_frequency_type;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Fixed dormant buying frequency",\
                      "doc": "The length in time steps of the dormant buying period. Required for fixed "\
                      "dormant_buying_frequency_type. Default is -1, agent has no dormant period and stays active.",\
                      "uitype": "range", \
                      "range": [-1, CY_LARGE_INT], \
                      "uilabel": "Dormant Buying Frequency Value"}
  int dormant_buying_val;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Dormant buying distribution minimum",\
                      "doc": "The minimum length in time steps of the dormant buying period. Required for Uniform and optional for "\
                      "Normal dormant_buying_frequency_type.",\
                      "uitype": "range", \
                      "range": [0, CY_LARGE_INT], \
                      "uilabel": "Dormant Buying Frequency Minimum"}
  int dormant_buying_min;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Dormant buying distribution maximum",\
                      "doc": "The maximum length in time steps of the dormant buying period. Required for "\
                      "Uniform dormant_buying_frequency_type, optional for Normal. Must be greater than or equal to dormant_buying_min ",\
                      "uitype": "range", \
                      "range": [0, CY_LARGE_INT], \
                      "uilabel": "Dormant Buying Frequency Maximum"}
  int dormant_buying_max;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Dormant buying distribution mean",\
                      "doc": "The mean length in time steps of the dormant buying period. Required for "\
                      "Normal dormant_buying_frequency_type. Must be greater than or equal to 0 ",\
                      "uitype": "range", \
                      "range": [0.0, CY_LARGE_DOUBLE], \
                      "uilabel": "Dormant Buying Frequency Mean"}
  double dormant_buying_mean;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Dormant buying distribution standard deviation",\
                      "doc": "The standard deviation of the length in time steps of the dormant buying period. "\
                      "Required for Normal dormant_buying_frequency_type. Must be greater than or equal to 0 ",\
                      "uitype": "range", \
                      "range": [0.0, CY_LARGE_DOUBLE], \
                      "uilabel": "Dormant Buying Frequency Standard Deviation"}
  double dormant_buying_stddev;

  #pragma cyclus var {"default": 0,\
                      "tooltip": "Probability that agent will return to active during the next time step",\
                      "doc": "Binomial distribution has a fixed probability of going active at any given "\
                      "timestep, like a weighted coin flip. Required for Binomial dormant_buying_frequency_type. "\
                      "Must be between 0 and 1",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Dormant Buying Binomial Offline Probability"}
  double dormant_buying_end_probability;

  #pragma cyclus var {"default": 0,\
                      "tooltip": "Probability that a cycle contains a disruption",\
                      "doc": "Probability that the agent undergoes a disruption (longer offline period) "\
                      "during any given cycle. Required for FixedWithDisruption dormant_buying_frequency_type.",\
                      "uitype": "range",\
                      "range": [0.0, 1.0],\
                      "uilabel": "Dormant Buying Disruption Probability"}
  double dormant_buying_disruption_probability;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Fixed length of disrupted cycle",\
                      "doc": "When a dormant cycle is disrupted, this is length of the offline period instead "\
                      "of dormant_buying_val. Required for FixedWithDisruption dormant_buying_frequency_type",\
                      "uitype": "range",\
                      "range": [0, CY_LARGE_INT]}
  int dormant_buying_disruption;

  #pragma cyclus var {"default": "Fixed",\
                      "tooltip": "Type of behavior used to determine size of buy request",\
                      "doc": "Behavior function used to determine the size of requests made. All values are "\
                      "a fraction of maximum capacity, determined by the throughput and capacity remaining."\
                      " Options: Fixed, Uniform, Normal. Fixed is default behavior. Uniform requires "\
                      "buying_size_min and buying_size_max. Normal requires "\
                      "buying_size_mean and buying_size_stddev, optional buying_size_min and "\
                      "buying_size_max.",\
                      "uitype": "combobox",\
                      "categorical": ["Fixed", "Uniform", "Normal"],\
                      "uilabel": "Buying Size Type"}
  std::string buying_size_type;

  #pragma cyclus var {"default": 1.0,\
                      "tooltip": "Fixed buying size",\
                      "doc": "The size of the buy request as a fraction of maximum capacity. Optional for Fixed "\
                      "buying_size_type. Must be greater than or equal to 0.0",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Buying Size Value"}
  double buying_size_val;

  #pragma cyclus var {"default": -1.0,\
                      "tooltip": "Buying size distribution minimum",\
                      "doc": "The minimum size of the buy request as a fraction of maximum capacity. "\
                      "Required for Uniform and optional for Normal buying_size_type. Must be greater than "\
                      "or equal to zero.",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Buying Size Minimum"}
  double buying_size_min;

  #pragma cyclus var {"default": -1.0,\
                      "tooltip": "Buying size distribution maximum",\
                      "doc": "The maximum size of the buy request as a fraction of maximum capacity. "\
                      "Required for Uniform buying_size_type, optional for Normal. Must be greater than "\
                      "or equal to buying_size_min ",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Buying Size Maximum"}
  double buying_size_max;

  #pragma cyclus var {"default": -1.0,\
                      "tooltip": "Buying size distribution mean",\
                      "doc": "The mean size of the buy request as a fraction of maximum capacity. "\
                      "Required for Normal buying_size_type.",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Buying Size Mean"}
  double buying_size_mean;

  #pragma cyclus var {"default": -1.0,\
                      "tooltip": "Buying size distribution standard deviation",\
                      "doc": "The standard deviation of the size of the buy request as a fraction of "\
                      "maximum capacity. Required for Normal buying_size_type.",\
                      "uitype": "range", \
                      "range": [0.0, 1.0], \
                      "uilabel": "Buying Size Standard Deviation"}
  double buying_size_stddev;

  #pragma cyclus var {"default": -1,\
                      "tooltip":"Reorder point",\
                      "doc":"The point at which the facility will request more material. "\
                      "Above this point, no request will be made. Must be less than max_inv_size."\
                      "If paired with reorder_quantity, this agent will have an (R,Q) inventory policy. "\
                      "If reorder_point is used alone, this agent will have an (s,S) inventory policy, "\
                      " with S (the maximum) being set at max_inv_size.",\
                      "uilabel":"Reorder Point"}
  double reorder_point;

  #pragma cyclus var {"default": -1,\
                      "tooltip":"Reorder amount (R,Q inventory policy)",\
                      "doc":"The amount of material that will be requested when the reorder point is reached. "\
                      "Exclusive request, so will demand exactly reorder_quantity."\
                      "Reorder_point + reorder_quantity must be less than max_inv_size.",\
                      "uilabel":"Reorder Quantity"}
  double reorder_quantity;

  #pragma cyclus var {"default": -1,\
                      "tooltip": "Total amount of material that can be recieved per cycle.",\
                      "doc": "After receiving this much material cumulatively, the agent will go dormant. "\
                      "Must be paired with dormant_buying_frequency_type and any other dormant parameters. "\
                      "The per-time step demand is unchanged except the cycle cap is almost reached.",\
                      "uilabel": "Cumulative Cap"}
  double cumulative_cap;

  #pragma cyclus var {"default": "unpackaged", \
                      "tooltip": "Output package", \
                      "doc": "Outgoing material will be packaged when trading.", \
                      "uitype": "package", \
                      "uilabel": "Package"}
  std::string package;

  #pragma cyclus var {"default": "unrestricted", \
                      "tooltip": "Output transport unit", \
                      "doc": "Outgoing material, after packaging, will be "\
                      "further restricted by transport unit when trading.", \
                      "uitype": "transportunit", \
                      "uilabel": "Transport Unit"}
  std::string transport_unit;

  #pragma cyclus var {"tooltip":"Incoming material buffer"}
  cyclus::toolkit::ResBuf<cyclus::Material> inventory;

  #pragma cyclus var {"tooltip":"Output material buffer"}
  cyclus::toolkit::ResBuf<cyclus::Material> stocks;

  #pragma cyclus var {"tooltip":"Buffer for material held for required residence_time"}
  cyclus::toolkit::ResBuf<cyclus::Material> ready;

  //// list of input times for materials entering the processing buffer
  #pragma cyclus var{"default": [],\
                      "internal": True}
  std::list<int> entry_times;

  #pragma cyclus var {"tooltip":"Buffer for material still waiting for required residence_time"}
  cyclus::toolkit::ResBuf<cyclus::Material> processing;

  #pragma cyclus var {"tooltip": "Total Inventory Tracker to restrict maximum agent inventory"}
  cyclus::toolkit::TotalInvTracker inventory_tracker;

  //// A policy for requesting material
  cyclus::toolkit::MatlBuyPolicy buy_policy;

  //// A policy for sending material
  cyclus::toolkit::MatlSellPolicy sell_policy;



  cyclus::IntDistribution::Ptr active_dist_ = NULL;
  cyclus::IntDistribution::Ptr dormant_dist_ = NULL;
  cyclus::DoubleDistribution::Ptr size_dist_ = NULL;

  cyclus::toolkit::Position coordinates;

  friend class StorageTest;

  // Adds required header to add geographic coordinates to the archetype
  #include "toolkit/position.cycpp.h"
};

}  // namespace cycamore

#endif  // CYCLUS_STORAGES_STORAGE_H_
