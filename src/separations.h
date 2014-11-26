#ifndef CYCAMORE_SRC_SEPARATIONS_H_
#define CYCAMORE_SRC_SEPARATIONS_H_

#include <string>

#include "cyclus.h"

namespace cycamore {

/// This model is intended to represent a generic separations process and is
/// based on the separations matrix facility from years past. Based on a matrix
/// of incoming streams, outgoing streams, and efficiencies, its purpose is to
/// convert incoming material into several different streams that can be
/// offered as differnent commodities after some period of delay.  Separations
/// is elemental in this case.  It was originally created to run the FCO
/// code-to-code comparison.
///
/// For realistic separations, the user is expected to produce an efficiency matrix
/// representing the separations technology of interest to them. By requesting the
/// feedstock from the appropriate markets, the facility acquires an unseparated
/// feedstock stream. Based on the input parameters in Table ref{tab:sepmatrix},
/// the separations process proceeds within the timesteps and other constraints of
/// the simulation.
///
/// Thereafter, separated streams as well as a stream of leftover material are
/// offered the appropriate markets for consumption by other facilities.
class Separations : public cyclus::Facility {
 public:  
  /// Constructor for Separations Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit Separations(cyclus::Context* ctx);

  #pragma cyclus

  #pragma cyclus note {"doc": "A separations facility is provided to separate "\
                              "materials into streams by element or nuclide."}

  /// A verbose printer for the Separations
  virtual std::string str();
  
  /// Handles the tick phase of the time step.
  virtual void Tick();

  /// makes requests of the in_commod
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory capacity, it will
  /// offer its minimum of its capacities.
  std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests);

  /// helper that makes bids for a commodity from a buffer
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests,
      std::string commod,
      cyclus::toolkit::ResourceBuff* buffer);  

  /// respond to each trade with a material of out_commod and out_recipe
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>, 
                          cyclus::Material::Ptr> >& responses);

  /// This facility accepts all materials of the right commodity
  virtual void AcceptMatlTrades(
      const std::vector<std::pair<cyclus::Trade<cyclus::Material>, 
                                  cyclus::Material::Ptr> >& responses);

  /// @param time the time of the tock
  virtual void Tock();

  /// Prints the status of the variables
  void PrintStatus();

  /// sets the in_commod variable
  inline void in_commod_(std::string c) {in_commod = c;};
  /// returns the in_commod variable
  inline std::string in_commod_() const {return in_commod;};

  /// sets the out_commods variable
  inline void out_commods_(std::vector< std::string > c) {out_commods = c;};
  /// returns the out_commods variable
  inline std::vector< std::string > out_commods_() const {return out_commods;};

  /// sets the waste_stream variable
  inline void waste_stream_(std::string c) {waste_stream = c;};
  /// returns the waste_stream variable
  inline std::string waste_stream_() const {return waste_stream;};

  /// sets the process_time variable
  inline void process_time_(int c) {process_time = c;};
  /// returns the process_time variable
  inline int process_time_() const {return process_time;};

  /// sets the max_inv_size variable
  inline void max_inv_size_(double c) {max_inv_size = c;};
  /// returns the max_inv_size variable
  inline double max_inv_size_() const {return max_inv_size;};

  /// sets the capacity variable
  inline void capacity_(double c) {capacity = c;};

  /// returns the capacity variable
  inline double capacity_() const {return capacity;};

  /// current maximum amount that can be added to processing
  inline double CurrentCapacity() const {
    return (std::min(capacity, max_inv_size - SepbuffQuantity()));
  } 

  /// sets the elems variable
  inline void elems_(std::vector<int> c) {elems = c;};

  /// returns the elems variable
  inline std::vector<int> elems_() const {return elems;};

  /// sets the effs variable
  inline void effs_(std::vector<std::string> c) {effs = c;};

  /// returns the effs variable
  inline std::vector<std::string> effs_() const {return effs;};

  /// sets the streams variable
  inline void streams_(std::vector<std::string> c) {streams = c;};

  /// returns the streams variable
  inline std::vector<std::string> streams_() const {return streams;};

  /// gives current quantity of all commods in sepbuff
  const double SepbuffQuantity() const;

  // gives current quantity of commod in sepbuff
  const double SepbuffQuantity(std::string commod) const;

  /// returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

protected:
  ///   adds a material into the incoming commodity sepbuff
  ///   @param mat the material to add to the incoming sepbuff.
  ///   @throws if there is trouble with pushing to the sepbuff buffer.
  void AddMat(cyclus::Material::Ptr mat);

  /// suggests, based on the buffer, a material response to an offer
  cyclus::Material::Ptr TradeResponse(double qty,
      cyclus::toolkit::ResourceBuff* buffer);
  
  /// Move all unprocessed sepbuff to processing
  void BeginProcessing(); 

  /// Separate all the material in processing
  void Separate();

  /// Separate all the material in the buff ResourceBuff
  /// @param buff the ResourceBuff to separate
  void Separate(cyclus::toolkit::ResourceBuff* buff);

  /// Separate a single material
  /// @param mat the material to separate
  void Separate(cyclus::Material::Ptr mat);

  /// returns the stream name for the element
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the stream name for the elemnt
  std::string Stream(int elem);

  /// returns the element in the matrix
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the separation efficiency for the element (0 <= x <= 1)
  double Eff(int elem);

  /// returns the index of the element in the vectors
  /// @param elem the integer representation of an element (e.g. 92)
  /// @return the index of the element in the lists
  int ElemIdx(int elem);

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip": "input commodities",\
                      "doc": "commodities accepted by this facility as "\
                             "feedstock for separations."}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"tooltip": "output stream list",\
                      "doc": "list of commodities produced by this facility"}
  std::vector<std::string> out_commods;

  #pragma cyclus var {"default": "separations_leftover",\
                      "tooltip": "waste stream name",\
                      "doc": "name of the commodity containing the losses"}
  std::string waste_commod;

  #pragma cyclus var {"default": 0,\
                      "tooltip": "process time (timesteps)",\
                      "doc": "the time it takes to convert a received commodity "\
                             "(timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip": "maximum sepbuff size (kg)",\
                      "doc": "the amount of material that can be in storage at "\
                             "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip": "maximum capacity (kg)",\
                      "doc": "the amount of material that can be processed per "\
                             "timestep (kg)."}
  double capacity; //should be nonnegative

  #pragma cyclus var {"tooltip": "elements to separate",\
                      "doc": "elements to separate"}
  std::vector<int> elems;

  #pragma cyclus var {"tooltip": "separation efficiencies",\
                      "doc": "number from 0.0 - 1.0, efficiency at separating "\
                             "each element."}
  std::vector<double> effs;

  #pragma cyclus var {"tooltip":"names of sepbuff streams",\
                      "doc":"string, for each sepbuff element, name the stream it "\
                      "belongs in. This list can contain repeated entries."}
  std::vector<std::string> streams;

  std::map<std::string, cyclus::toolkit::ResourceBuff> sepbuff_;
  cyclus::toolkit::ResourceBuff rawbuff_;
  cyclus::toolkit::ResourceBuff wastes_;

  /// a list of preferred commodities
  std::map<int, std::set<std::string> > prefs_;

  /// map from ready time to resource buffers
  std::map<int, cyclus::toolkit::ResourceBuff> processing;

  friend class SeparationsTest;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SEPARATIONS_H_
