#ifndef CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_H_
#define CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_H_

#include <string>

#include "cyclus.h"

// forward declaration
namespace commodconverter {
class CommodConverter;
} // namespace commodconverter


namespace commodconverter {
/// @class CommodConverter
///
/// This Facility is intended to convert a resource from one commodity to 
/// another. It also has an optional delay parameter. It can therefore be used 
/// quite easily as a storage facility. 
/// The CommodConverter class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// This Agent was initially developed to support the fco code-to-code 
/// comparsion.
/// It's very similar to the "NullFacility" of years 
/// past. Its purpose is to convert a commodity from one commodity to another 
/// after some period of delay time. This facility is very good for use as a 
/// storage facility or as a facility whose process physics will not transmute 
/// the material composition any more than with normal, system-wide decay. 
///
/// @section agentparams Agent Parameters
/// in_commod is a string naming the commodity that this facility recieves
/// out_commod is a string naming the commodity that in_commod is converted into
/// process_time is the number of timesteps between receiving and offering
/// 
/// @section optionalparams Optional Parameters
/// max_inv_size is the maximum capacity of the inventory storage
/// capacity is the maximum processing capacity per timestep
///
/// @section detailed Detailed Behavior
/// 
/// Tick:
/// Nothing really happens on the tick. 
///
/// Tock:
/// On the tock, any material that has been waiting for long enough (delay 
/// time) is converted and placed in the stocks buffer.
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
/// Any converted material in the stocks buffer is offered to the market.
///
/// Sending Resources:
/// Matched resources are sent immediately.
class CommodConverter 
  : public cyclus::Facility,
    public cyclus::toolkit::CommodityProducer {
 public:  
  /// Constructor for CommodConverter Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  CommodConverter(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A commodconverter facility converts from one " \
                              "commodity to another, with an optional delay."}

  /// A verbose printer for the CommodConverter
  virtual std::string str();

  // --- Facility Members ---
  
  // --- Agent Members ---
  virtual void EnterNotify();

  /// The handleTick function specific to the CommodConverter.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the CommodConverter.
  /// @param time the time of the tock
  virtual void Tock();

  /// @brief The CommodConverter request Materials of its given
  /// commodity.
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// @brief The CommodConverter place accepted trade Materials in their
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory capacity, it will
  /// offer its minimum of its capacities.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  /// @brief respond to each trade with a material enriched to the appropriate
  /// level given this facility's inventory
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

  /* --- */

  /* --- CommodConverter Members --- */

  /* --- */

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

  /// @brief Convert one ready resource in processing
  void Convert_();

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }

  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /// @brief returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::string in_commod;
  inline std::string in_commod_() const {return in_commod;};

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility"}
  std::string out_commod;
  inline std::string out_commod_() const {return out_commod;};

  #pragma cyclus var {"tooltip":"input recipe",\
                      "doc":"recipe accepted by this facility"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var{"default": 1e299,\
                     "tooltip":"capacity per timestep (kg)",\
                     "doc":"the maximumamount that can be processed per timestep (kg)"}
  double capacity;

  cyclus::toolkit::ResourceBuff inventory;
  cyclus::toolkit::ResourceBuff stocks;

  /// @brief map from ready time to resource buffers
  std::map<int, cyclus::toolkit::ResourceBuff> processing;

  cyclus::toolkit::CommodityRecipeContext crctx_;

  /// @brief the processing time required for a full process
  inline void process_time_(int t) { process_time = t; }
  inline int process_time_() const { return process_time; }

  /// @brief the maximum amount allowed in inventory
  inline void capacity_(double c) { max_inv_size = c; }
  inline double capacity_() const { return max_inv_size; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const { 
    return max_inv_size - inventory.quantity(); }

  friend class CommodConverterTest;
};

}  // namespace commodconverter

#endif  // CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_H_
