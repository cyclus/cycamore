#ifndef CYCLUS_STORAGES_STORAGE_H_
#define CYCLUS_STORAGES_STORAGE_H_

#include <string>
#include <list>
#include <vector>

#include "cyclus.h"

// forward declaration
namespace storage {
class Storage;
} // namespace storage


namespace storage {
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
/// max_inv_size is the maximum capacity of the inventory storage
/// throughput is the maximum processing capacity per timestep
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

  #pragma cyclus note {"doc": "A storage facility converts from one " \
                              "commodity to another, with an optional delay."}

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

  /* --- Storage Members --- */

  /// @brief the minimum processing time required for a full process
  inline void residence_time_(int t) { residence_time = t; }
  inline int residence_time_() const { return residence_time; }

  /// @brief the maximum amount allowed in inventory
  inline void max_inv_size_(double c) { max_inv_size = c; }
  inline double max_inv_size_() const { return max_inv_size; }

  /// @brief the maximum amount processed per timestep
  inline void throughput_(double c) { throughput = c; }
  inline double throughput_() const { return throughput; }

  /// @brief the in commodity
  inline void in_commods_(std::string c) { in_commods.push_back(c); }
  inline std::vector<std::string> in_commods_() const { return in_commods; }

  /// @brief the in commodity preferences
  inline void in_commod_prefs_(double c) { in_commod_prefs.push_back(c); }
  inline double in_commod_prefs_() const { return in_commod_prefs.front(); }

  /// @brief the out commodity
  inline void out_commods_(std::string c) { out_commods.push_back(c); }
  inline std::string out_commods_() const {
    if (out_commods.size() == 0){
      return "out_commod";
    }
     return out_commods.front(); }

  /// @brief the in recipe
  inline void in_recipe_(std::string c) { in_recipe = c; }
  inline std::string in_recipe_() const { return in_recipe; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const { 
    return (max_inv_size - processing.quantity() - stocks.quantity()); }

  /// @brief the batch handling identifier
  inline void batch_handling_(bool c) { batch_handling = c; }
  inline bool batch_handling_() const { return batch_handling; }

  /// @brief returns the time key for ready materials
  int ready_time(){ return context()->time() - residence_time; }

 protected:
  ///   @brief adds a material into the incoming commodity inventory
  ///   @param mat the material to add to the incoming inventory.
  ///   @throws if there is trouble with pushing to the inventory buffer.
  void AddMat_(cyclus::Material::Ptr mat);

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_();

  /// @brief Move as many ready resources as allowable into stocks
  /// @param cap current throughput capacity 
  void ProcessMat_(double cap);

  /// @brief any ready resources remaining in processing get pushed off to next timestep
  /// @param time the timestep whose buffer remains unprocessed 
  // void AdvanceUnconverted_(int time);

  /// @brief move ready resources from processing to ready at a certain time
  /// @param time the time of interest
  void ReadyMatl_(int time);

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"default": [1]}
  std::vector<double> in_commod_prefs;

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility"}
  std::vector<std::string> out_commods;

  #pragma cyclus var {"default":"",\
                      "tooltip":"input recipe",\
                      "doc":"recipe accepted by this facility"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"residence time (timesteps)",\
                      "doc":"the minimum holding time for a received commodity (timesteps)."}
  int residence_time;

  #pragma cyclus var {"default": 1e299,\
                     "tooltip":"throughput per timestep (kg)",\
                     "doc":"the max amount that can be processed per timestep (kg)"}
  double throughput;

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage"}
  double max_inv_size; 

  #pragma cyclus var {"default": True,\
                      "tooltip":"Bool to determine how Storage handles batches",\
                      "doc":"Determines if Storage will divide resource objects {discrete} or {continuous}. "\
                      "True for discrete, false for continuous. Default to discrete"}
  bool batch_handling;                    

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

  cyclus::toolkit::ResBuf<cyclus::Material> processing;

  //// A policy for requesting material
  cyclus::toolkit::MatlBuyPolicy buy_policy;

  //// A policy for sending material
  cyclus::toolkit::MatlSellPolicy sell_policy;


  friend class StorageTest;
};

}  // namespace storage

#endif  // CYCLUS_STORAGES_STORAGE_H_
