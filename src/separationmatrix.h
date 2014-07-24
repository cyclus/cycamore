#ifndef CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_FACILITY_H_
#define CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_FACILITY_H_

#include <string>

#include "cyclus.h"

namespace separationmatrix {


/// @class SeparationmatrixFacility
///
/// This Facility is intended
/// as a skeleton to guide the implementation of new Facility
/// agents.
/// The SeparationmatrixFacility class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
///  For realistic separations, the user is expected to produce an efficiency matrix
///  representing the separations technology of interest to them. By requesting the
///  feedstock from the appropriate markets, the facility acquires an unseparated
///  feedstock stream. Based on the input parameters in Table ref{tab:sepmatrix},
///  the separations process proceeds within the timesteps and other constraints of
///  the simulation.
///  
///  Thereafter, separated streams as well as a stream of losses are offered the
///  appropriate markets for consumption by other facilities. In the transition
///  scenario at hand, the StreamBlender fuel fabrication facility purchases the
///  streams it desires in order to produce SFR fuel.
///
/// @section agentparams Agent Parameters
/// Input Commodity
/// An ElementGrouping map
/// 
/// @section optionalparams Optional Parameters
/// 
/// @section detailed Detailed Behavior
///  TICK
///  Make offers of separated material based on availabe inventory.
///  If there are ordersWaiting, prepare and send an appropriate
///  request for spent fuel material.
///  Check stocks to determine if there is capacity to produce any extra material
///  next month. If so, process as much raw (spent fuel) stock material as
///  capacity will allow.
///
///  TOCK
///  Send appropriate separated material from inventory to fill ordersWaiting.
///
///  RECIEVE MATERIAL
///  Put incoming spent nuclear fuel (SNF) material into stocks
///
///  SEND MATERIAL
///  Pull separated material from inventory based on Requests
///  Decrement ordersWaiting
/// Place a description of the detailed behavior of the agent. Consider
/// describing the behavior at the tick and tock as well as the behavior
/// upon sending and receiving materials and messages.
class SeparationmatrixFacility : public cyclus::Facility  {
 public:  
  /// Constructor for SeparationmatrixFacility Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit SeparationmatrixFacility(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus

  #pragma cyclus note {"doc": "A separationmatrix facility is provided as a skeleton " \
                              "for the design of new facility agents."}

  /// A verbose printer for the SeparationmatrixFacility
  virtual std::string str();
  
  /// The handleTick function specific to the SeparationmatrixFacility.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the SeparationmatrixFacility.
  /// @param time the time of the tock
  virtual void Tock();

protected:
  // @brief gives current quantity of commod in inventory
  const double inventory_quantity(std::string commod) const;

  /// @brief gives current quantity of all commods in inventory
  const double inventory_quantity() const;

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }
  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_(); 

  /// @brief Separate all the material in processing
  void Separate_();

  /// @brief Separate all the material in the buff ResourceBuff
  /// @param buff the ResourceBuff to separate
  void Separate_(cyclus::toolkit::ResourceBuff buff);

  /// @brief Separate a single material
  /// @param mat the material to separate
  void Separate_(cyclus::Material::Ptr mat);

  /// @brief returns the stream name for the element
  std::string Stream_(int elem);

  /// @brief returns the element in the matrix
  double Eff_(int elem);

  /// @brief returns the index of the element in the vectors
  int ElemIdx_(int elem);

  /// @brief returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

  /* --- Module Members --- */
  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::string in_commod;
  inline std::string in_commod_() const {return in_commod;};

  #pragma cyclus var {"tooltip":"output stream list     ",\
                      "doc":"list of commodities produced by this facility"}
  std::vector< std::string > out_commods;
  inline std::vector< std::string > out_commods_() const {return out_commods;};

  #pragma cyclus var {"default":"losses",\
                      "tooltip":"waste (losses) stream name",\
                      "doc":"name of the commodity containing the losses"}
  std::string waste_stream;
  inline std::string waste_stream_() const {return waste_stream;};

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var{'capacity': 'max_inv_size'}
  std::map<std::string, cyclus::toolkit::ResourceBuff> inventory;
  cyclus::toolkit::ResourceBuff stocks;
  cyclus::toolkit::ResourceBuff wastes;

  #pragma cyclus var {"tooltip":"elements to separate",\
                      "doc":"elements to separate"}
  std::vector<int> elems;
  inline std::vector<int> elems_() const {return elems;};

  #pragma cyclus var {"tooltip":"separation efficiencies",\
                      "doc":"double, in the form of a string (because of the db). "\
                      "number from 0-1, efficiency at separating each element."}
  std::vector<std::string> effs;
  inline std::vector<std::string> effs_() const {return effs;};

  #pragma cyclus var {"tooltip":"names of separated streams",\
                      "doc":"string, for each separated element, name the stream it "\
                      "belongs in. This list can contain repeated entries."}
  std::vector<std::string> streams;
  inline std::vector<std::string> streams_() const {return streams;};

  /// @brief a list of preffered commodities
  std::map<int, std::set<std::string> > prefs_;

  /// @brief map from ready time to resource buffers
  std::map<int, cyclus::toolkit::ResourceBuff> processing;

  cyclus::toolkit::CommodityRecipeContext crctx_;

  /// @brief the processing time required for a full process
  inline void process_time_(int t) { process_time = t; }
  inline int process_time_() const { return process_time; }

  /// @brief the maximum amount allowed in inventory
  inline void capacity(double c) { max_inv_size = c; }
  inline double capacity() const { return max_inv_size; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const {
    return (max_inv_size - inventory_quantity()); } 
};

}  // namespace separationmatrix

#endif  // CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_FACILITY_H_
