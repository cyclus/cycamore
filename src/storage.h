#ifndef CYCLUS_STORAGES_STORAGE_H_
#define CYCLUS_STORAGES_STORAGE_H_

#include <string>

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
/// process time or holdup time.
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
/// in_commod is a string naming the commodity that this facility receives
/// out_commod is a string naming the commodity that in_commod is stocks into
/// process_time is the number of timesteps between receiving and offering
/// in_recipe (optional) describes the incoming resource by recipe
/// 
/// @section optionalparams Optional Parameters
/// max_inv_size is the maximum capacity of the inventory storage
/// capacity is the maximum processing capacity per timestep
///
/// @section detailed Detailed Behaviour
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
  /// Constructor for Storage Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  Storage(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A storage facility converts from one " \
                              "commodity to another, with an optional delay."}

  /// A verbose printer for the Storage Facility
  virtual std::string str();

  // --- Facility Members ---
  
  // --- Agent Members ---
  virtual void EnterNotify();

  /// The handleTick function specific to the Storage Facility.
  virtual void Tick();

  /// The handleTick function specific to the Storage Facility.
  virtual void Tock();

  /// @brief The Storage Facility requests Materials of its desired
  /// commodity.
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// @brief The Storage Facility places accepted trade Materials into
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory capacity, it will
  /// offer the minimum of its capacities.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  /// @brief respond to each trade with a material of out_commod and out_recipe
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

  /* --- */

  /* --- Storage Members --- */

  /* --- */
  /// @brief the processing time required for a full process
  inline void process_time_(int t) { process_time = t; }
  inline int process_time_() const { return process_time; }

  /// @brief the maximum amount allowed in inventory
  inline void max_inv_size_(double c) { max_inv_size = c; }
  inline double max_inv_size_() const { return max_inv_size; }

  /// @brief the maximum amount processed per timestep
  inline void capacity_(double c) { capacity = c; }
  inline double capacity_() const { return capacity; }

  /// @brief the cost per unit out_commod
  inline void cost_(double c) { cost = c; }
  inline double cost_() const { return cost; }

  /// @brief the in commodity
  inline void in_commod_(std::string c) { in_commod = c; }
  inline std::string in_commod_() const { return in_commod; }

  /// @brief the out commodity
  inline void out_commod_(std::string c) { out_commod = c; }
  inline std::string out_commod_() const { return out_commod; }

  /// @brief the in recipe
  inline void in_recipe_(std::string c) { in_recipe = c; }
  inline std::string in_recipe_() const { return in_recipe; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const { 
    return std::min(capacity, max_inv_size - inventory.quantity()); }

  /// @brief returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

 protected:
  ///   @brief adds a material into the incoming commodity inventory
  ///   @param mat the material to add to the incoming inventory.
  ///   @throws if there is trouble with pushing to the inventory buffer.
  void AddMat_(cyclus::Material::Ptr mat);

  ///   @brief generates a request for this facility given its current state. The
  ///   quantity of the material will be equal to the remaining inventory size.
  ///   @return a material that this facility will request
  cyclus::Material::Ptr Request_();

  /// @brief gathers information about bids
  /// @param commod_requests the materials that have been requested
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids_(
        cyclus::CommodMap<cyclus::Material>::type& commod_requests, 
        std::string commod, 
        cyclus::toolkit::ResourceBuff* buffer);

  /// @brief suggests, based on the buffer, a material response to an offer
  cyclus::Material::Ptr TradeResponse_(
      double qty, 
      cyclus::toolkit::ResourceBuff* buffer);

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_();

  /// @brief Move one ready resource in processing
  /// @param cap current conversion capacity 
  void ProcessMat_(double cap);

  /// @brief any ready resources in processing get pushed off to next timestep
  /// @param time the timestep whose buffer remains unprocessed 
  void AdvanceUnconverted_(int time);

  /// @brief report the resource quantity in processing at a certain time
  /// @param time the time of interest
  double ProcessingAmt_(int time);

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }

  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::string in_commod;

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility"}
  std::string out_commod;

  #pragma cyclus var {"default":"",\
                      "tooltip":"input recipe",\
                      "doc":"recipe accepted by this facility"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time;

  #pragma cyclus var {"default": 1e299,\
                     "tooltip":"capacity per timestep (kg)",\
                     "doc":"the max amount that can be processed per timestep (kg)"}
  double capacity;

  #pragma cyclus var {"default": 0,\
                     "tooltip":"cost per kg of production",\
                     "doc":"cost per kg of produced out_commod"}
  double cost;

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage"}
  double max_inv_size; 


  cyclus::toolkit::ResourceBuff inventory;
  cyclus::toolkit::ResourceBuff stocks;

  /// @brief map from ready time to resource buffers
  std::map<int, cyclus::toolkit::ResourceBuff> processing;

  cyclus::toolkit::CommodityRecipeContext crctx_;


  friend class StorageTest;
};

}  // namespace storage

#endif  // CYCLUS_STORAGES_STORAGE_H_
