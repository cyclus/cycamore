#ifndef CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
#define CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_

#include <string>

#include "cyclus.h"

// forward declaration
namespace commodconverter {
class CommodconverterFacility;
} // namespace commodconverter


namespace commodconverter {
/// @class CommodconverterFacility
///
/// This Facility is intended
/// as a skeleton to guide the implementation of new Facility
/// agents.
/// The CommodconverterFacility class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// Place an introduction to the agent here.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///
/// @section optionalparams Optional Parameters
/// Place a description of the optional input parameters to define the
/// agent implementation.
///
/// @section detailed Detailed Behavior
/// Place a description of the detailed behavior of the agent. Consider
/// describing the behavior at the tick and tock as well as the behavior
/// upon sending and receiving materials and messages.
class CommodconverterFacility 
  : public cyclus::Facility,
    public cyclus::toolkit::CommodityProducer {
 public:  
  /// Constructor for CommodconverterFacility Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  CommodconverterFacility(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A commodconverter facility converts from one " \
                              "commodity to another, with an optional delay."}

  /// A verbose printer for the CommodconverterFacility
  virtual std::string str();

  /* --- Facility Members --- */
  /* --- */
  
  /* --- Agent Members --- */
  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tock
  virtual void Tock();

  /// @brief The CommodconverterFacility request Materials of its given
  /// commodity.
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// @brief The CommodconverterFacility place accepted trade Materials in their
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

  /* --- CommodconverterFacility Members --- */

  /* --- */

 protected:
  ///   @brief adds a material into the incoming commodity inventory
  ///   @throws if the material is not the same composition as the in_recipe
  void AddMat_(cyclus::Material::Ptr mat);

  ///   @brief generates a request for this facility given its current state. The
  ///   quantity of the material will be equal to the remaining inventory size.
  cyclus::Material::Ptr Request_();

  /// @brief gathers information about bids
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

  #pragma cyclus var {"tooltip":"output recipe",\
                      "doc":"recipe produced by this facility"}
  std::string out_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var{'capacity': 'max_inv_size_'}
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

  friend class CommodconverterFacilityTest;
};

}  // namespace commodconverter

#endif  // CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
