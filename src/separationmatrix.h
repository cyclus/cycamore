#ifndef CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_H_
#define CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_H_

#include <string>

#include "cyclus.h"

namespace separationmatrix {


/// @class SeparationMatrix
///
/// This model is intended to represent a generic separations process and is based 
/// on the separations matrix facility from years past. Based on a matrix of 
/// incoming streams, outgoing streams, and efficiencies, its purpose is to 
/// convert a commodity from one commodity to a few after some period of delay 
/// time. Separations is elemental in this case. This facility is very good for 
/// use as a separations facility in a reprocessing scenario. It was recently 
/// re-created to run the fco code-to-code comparison.
/// 
/// The SeparationMatrix class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
///  For realistic separations, the user is expected to produce an efficiency matrix
///  representing the separations technology of interest to them. By requesting the
///  feedstock from the appropriate markets, the facility acquires an unsepbuff
///  feedstock stream. Based on the input parameters in Table ref{tab:sepmatrix},
///  the separations process proceeds within the timesteps and other constraints of
///  the simulation.
///  
///  Thereafter, sepbuff streams as well as a stream of losses are offered the
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
///  Make offers of sepbuff material based on availabe sepbuff.
///  If there are ordersWaiting, prepare and send an appropriate
///  request for spent fuel material.
///  Check rawbuff to determine if there is capacity to produce any extra material
///  next month. If so, process as much raw (spent fuel) stock material as
///  capacity will allow.
///
///  TOCK
///  Send appropriate sepbuff material from sepbuff to fill ordersWaiting.
///
///  RECIEVE MATERIAL
///  Put incoming spent nuclear fuel (SNF) material into rawbuff
///
///  SEND MATERIAL
///  Pull sepbuff material from sepbuff based on Requests
///  Decrement ordersWaiting
class SeparationMatrix : 
  public cyclus::Facility,
  public cyclus::toolkit::CommodityProducer {
 public:  
  /// Constructor for SeparationMatrix Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit SeparationMatrix(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A separationmatrix facility is provided to "\
                              "separate materials into streams by element "}

  /// Notify the simulation that this facility has arrived
  void EnterNotify();

  /// A verbose printer for the SeparationMatrix
  virtual std::string str();
  
  /// The handleTick function specific to the SeparationMatrix.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the SeparationMatrix.
  /// @param time the time of the tock
  virtual void Tock();

  /// @brief makes requests of the in_commod
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> 
    GetMatlRequests();

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory capacity, it will
  /// offer its minimum of its capacities.
  std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
    GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                                  commod_requests);

  /// @brief makes bids for a commodity from a buffer
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids_(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests,
      std::string commod,
      cyclus::toolkit::ResourceBuff* buffer);  

  /// This facility accepts all materials of the right commodity
  virtual void AcceptMatlTrades( const std::vector< 
      std::pair<cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& 
      responses);

  /// @brief respond to each trade with a material of out_commod and out_recipe
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

  /// Prints the status of the variables
  void PrintStatus();

  /// @brief sets the in_commod variable
  inline void in_commod_(std::string c) {in_commod = c;};
  /// @brief returns the in_commod variable
  inline std::string in_commod_() const {return in_commod;};

  /// @brief sets the out_commods variable
  inline void out_commods_(std::vector< std::string > c) {out_commods = c;};
  /// @brief returns the out_commods variable
  inline std::vector< std::string > out_commods_() const {return out_commods;};

  /// @brief sets the waste_stream variable
  inline void waste_stream_(std::string c) {waste_stream = c;};
  /// @brief returns the waste_stream variable
  inline std::string waste_stream_() const {return waste_stream;};

  /// @brief sets the process_time variable
  inline void process_time_(int c) {process_time = c;};
  /// @brief returns the process_time variable
  inline int process_time_() const {return process_time;};

  /// @brief sets the max_inv_size variable
  inline void max_inv_size_(double c) {max_inv_size = c;};
  /// @brief returns the max_inv_size variable
  inline double max_inv_size_() const {return max_inv_size;};

  /// @brief sets the capacity variable
  inline void capacity_(double c) {capacity = c;};
  /// @brief returns the capacity variable
  inline double capacity_() const {return capacity;};

  /// @brief the cost per unit out_commod
  inline void cost_(double c) { cost = c; }
  /// @brief returns the cost variable
  inline double cost_() const { return cost; } 

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const {
    return (std::min(capacity, max_inv_size - sepbuff_quantity())); } 

  /// @brief sets the elems variable
  inline void elems_(std::vector<int> c) {elems = c;};
  /// @brief returns the elems variable
  inline std::vector<int> elems_() const {return elems;};

  /// @brief sets the effs variable
  inline void effs_(std::vector<std::string> c) {effs = c;};
  /// @brief returns the effs variable
  inline std::vector<std::string> effs_() const {return effs;};

  /// @brief sets the streams variable
  inline void streams_(std::vector<std::string> c) {streams = c;};
  /// @brief returns the streams variable
  inline std::vector<std::string> streams_() const {return streams;};

  // @brief gives current quantity of commod in sepbuff
  const double sepbuff_quantity(std::string commod) const;

  /// @brief gives current quantity of all commods in sepbuff
  const double sepbuff_quantity() const;

  /// @brief returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

protected:
  ///   @brief adds a material into the incoming commodity sepbuff
  ///   @param mat the material to add to the incoming sepbuff.
  ///   @throws if there is trouble with pushing to the sepbuff buffer.
  void AddMat_(cyclus::Material::Ptr mat);

  /// @brief suggests, based on the buffer, a material response to an offer
  cyclus::Material::Ptr TradeResponse_(
      double qty,
      cyclus::toolkit::ResourceBuff* buffer);
  
  /// @brief registers the commodity production for this facility
  /// @param commod_str a commodity that this facility produces
  /// @param cap the capacity of this facility to produce the commod
  /// @param cost the cost of the commods
  void RegisterProduction(std::string commod_str, double cap, double cost);

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }
  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /// @brief Move all unprocessed sepbuff to processing
  void BeginProcessing_(); 

  /// @brief Separate all the material in processing
  void Separate_();

  /// @brief Separate all the material in the buff ResourceBuff
  /// @param buff the ResourceBuff to separate
  void Separate_(cyclus::toolkit::ResourceBuff* buff);

  /// @brief Separate a single material
  /// @param mat the material to separate
  void Separate_(cyclus::Material::Ptr mat);

  /// @brief returns the stream name for the element
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the stream name for the elemnt
  std::string Stream_(int elem);

  /// @brief returns the element in the matrix
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the separation efficiency for the element (0 <= x <= 1)
  double Eff_(int elem);

  /// @brief returns the index of the element in the vectors
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the index of the element in the lists
  int ElemIdx_(int elem);

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::string in_commod;

  #pragma cyclus var {"tooltip":"output stream list     ",\
                      "doc":"list of commodities produced by this facility"}
  std::vector< std::string > out_commods;

  #pragma cyclus var {"default":"losses",\
                      "tooltip":"waste (losses) stream name",\
                      "doc":"name of the commodity containing the losses"}
  std::string waste_stream;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum sepbuff size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum capacity (kg)",\
                      "doc":"the amount of material that can be processed per "\
                      "timestep (kg)."}
  double capacity; //should be nonnegative

  #pragma cyclus var {"default": 0,\
                     "tooltip":"cost per kg of production",\
                     "doc":"cost per kg of produced material"}
  double cost;

  #pragma cyclus var {"tooltip":"elements to separate",\
                      "doc":"elements to separate"}
  std::vector<int> elems;

  #pragma cyclus var {"tooltip":"separation efficiencies",\
                      "doc":"double, in the form of a string (because of the db). "\
                      "number from 0-1, efficiency at separating each element."}
  std::vector<std::string> effs;

  #pragma cyclus var {"tooltip":"names of sepbuff streams",\
                      "doc":"string, for each sepbuff element, name the stream it "\
                      "belongs in. This list can contain repeated entries."}
  std::vector<std::string> streams;

  std::map<std::string, cyclus::toolkit::ResourceBuff> sepbuff;
  cyclus::toolkit::ResourceBuff rawbuff;
  cyclus::toolkit::ResourceBuff wastes;


  /// @brief a list of preffered commodities
  std::map<int, std::set<std::string> > prefs_;

  /// @brief map from ready time to resource buffers
  std::map<int, cyclus::toolkit::ResourceBuff> processing;

  cyclus::toolkit::CommodityRecipeContext crctx_;

  friend class SeparationMatrixTest;
};

}  // namespace separationmatrix

#endif  // CYCLUS_SEPARATIONMATRIXS_SEPARATIONMATRIX_H_
