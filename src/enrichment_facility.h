#ifndef CYCAMORE_SRC_ENRICHMENT_FACILITY_H_
#define CYCAMORE_SRC_ENRICHMENT_FACILITY_H_

#include <string>

#include "cyclus.h"

namespace cycamore {

/// @class SWUConverter
///
/// @brief The SWUConverter is a simple Converter class for material to
/// determine the amount of SWU required for their proposed enrichment
class SWUConverter : public cyclus::Converter<cyclus::Material> {
 public:
  SWUConverter(double feed, double tails) : feed_(feed), tails_(tails) {}
  virtual ~SWUConverter() {}

  /// @brief provides a conversion for the SWU required
  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material>
          const * ctx = NULL) const {
    cyclus::toolkit::Assays assays(feed_, cyclus::toolkit::UraniumAssay(m),
                                   tails_);
    return cyclus::toolkit::SwuRequired(m->quantity(), assays);
  }

  /// @returns true if Converter is a SWUConverter and feed and tails equal
  virtual bool operator==(Converter& other) const {
    SWUConverter* cast = dynamic_cast<SWUConverter*>(&other);
    return cast != NULL &&
    feed_ == cast->feed_ &&
    tails_ == cast->tails_;
  }

 private:
  double feed_, tails_;
};

/// @class NatUConverter
///
/// @brief The NatUConverter is a simple Converter class for material to
/// determine the amount of natural uranium required for their proposed
/// enrichment
class NatUConverter : public cyclus::Converter<cyclus::Material> {
 public:
  NatUConverter(double feed, double tails) : feed_(feed), tails_(tails) {}
  virtual ~NatUConverter() {}

  /// @brief provides a conversion for the amount of natural Uranium required
  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material>
          const * ctx = NULL) const {
    cyclus::toolkit::Assays assays(feed_, cyclus::toolkit::UraniumAssay(m),
                                   tails_);
    return cyclus::toolkit::FeedQty(m->quantity(), assays);
  }

  /// @returns true if Converter is a NatUConverter and feed and tails equal
  virtual bool operator==(Converter& other) const {
    NatUConverter* cast = dynamic_cast<NatUConverter*>(&other);
    return cast != NULL &&
    feed_ == cast->feed_ &&
    tails_ == cast->tails_;
  }

 private:
  double feed_, tails_;
};

///  @class EnrichmentFacility
///
///  @section introduction Introduction
///  The EnrichmentFacility is a simple Agent to agent the enriching of natural
///  Uranium in a Cyclus simulation. It requests its input recipe (nominally
///  natural Uranium), and produces any amount of enriched Uranium, given the its
///  natural uranium inventory constraint and its SWU capacity constraint.
///
///  @section requests Requests
///  The EnrichmentFacility will request from the cyclus::ResourceExchange a
///  cyclus::Material whose quantity is its remaining inventory capacity and whose
///  composition is that of its input recipe.
///
///  @section acctrade Accepting Trades
///  The EnrichmentFacility adds any accepted trades to its inventory.
///
///  @section bids Bids
///  The EnrichmentFacility will bid on any request for its output commodity. It
///  will bid either the request quantity, or the quanity associated with either
///  its SWU constraint or natural uranium constraint, whichever is lower.
///
///  @section extrades Executing Trades
///  The EnrichmentFacility will execute trades for its output commodity in the
///  following manner:
///    #. Determine the trade's quantity and product assay
///    #. Determine the natural Uranium and SWU requires to create that product
///    #. Remove the required quantity of natural Uranium from its inventory
///    #. Extract the appropriate composition of enriched Uranium
///    #. Send the enriched Uranium as the trade resource
///
///  @section gotchas Gotchas
///  #. In its current form, the EnrichmentFacility can only accept
///  cyclus::Material having the composition of its input recipe. If a
///  cyclus::Material of a different composition is sent to it, an exception will
///  be thrown.
///
///  #. During the trading phase, an exception will be thrown if either the
///  EnrichmentFacility's SWU or inventory constraint is breached.
///
///  @section improvements Improvments
///  The primary improvement to the EnrichmentFacility would be to relax the
///  requirement that all input material have the in_recipe composition (i.e.,
///  allow different base enrichments of Uranium).
///
///  How would I go about doing so? I'd likely develop an EnrichmentBuffer-type
///  class that can be queried as to its SWU and natural Uranium capacity.
class EnrichmentFacility : public cyclus::Facility {
 public:
  // --- Module Members ---
///    Constructor for the EnrichmentFacility class
///    @param ctx the cyclus context for access to simulation-wide parameters
  EnrichmentFacility(cyclus::Context* ctx);

///     Destructor for the EnrichmentFacility class
  virtual ~EnrichmentFacility();

  #pragma cyclus decl

  #pragma cyclus note {"doc": "An enrichment facility that intakes a commodity " \
                              "(usually natural uranium) and supplies a user-" \
                              "specified enriched product based on SWU capacity"}

///     Print information about this agent
  virtual std::string str();
  // ---

  // --- Facility Members ---
  /// perform module-specific tasks when entering the simulation
  virtual void Build(cyclus::Agent* parent);
  // ---

  // --- Agent Members ---
  ///  Each facility is prompted to do its beginning-of-time-step
  ///  stuff at the tick of the timer.

  ///  @param time is the time to perform the tick
  virtual void Tick();

  ///  Each facility is prompted to its end-of-time-step
  ///  stuff on the tock of the timer.

  ///  @param time is the time to perform the tock
  virtual void Tock();

  /// @brief The EnrichmentFacility request Materials of its given
  /// commodity.
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  /// @brief The EnrichmentFacility place accepted trade Materials in their
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory or SWU capacity, it will
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
  // ---

  // --- EnrichmentFacility Members ---
  ///  @brief Determines if a particular material is a valid request to respond
  ///  to.  Valid requests must contain U235 and U238 and must have a relative
  ///  U235-to-U238 ratio less than this facility's tails_assay().
  ///  @return true if the above description is met by the material
  bool ValidReq(const cyclus::Material::Ptr mat);

  inline void in_commodity(std::string in_com) { in_commod = in_com; }

  inline std::string in_commodity() const { return in_commod; }

  inline void out_commodity(std::string out_com) {
    out_commod = out_com;
  }

  inline std::string out_commodity() const { return out_commod; }

  inline void InRecipe(std::string in_rec) { in_recipe = in_rec; }

  inline std::string InRecipe() const { return in_recipe; }

  inline void SetMaxInventorySize(double size) {
    max_inv_size = size;
    inventory.set_capacity(size);
  }

  inline double MaxInventorySize() const { return inventory.capacity(); }

  inline double InventorySize() const { return inventory.quantity(); }

  inline void FeedAssay(double assay) { feed_assay = assay; }

  inline double FeedAssay() const { return feed_assay; }

  inline void TailsAssay(double assay) { tails_assay = assay; }

  inline double TailsAssay() const { return tails_assay; }

  inline void SwuCapacity(double capacity) {
    swu_capacity = capacity;
    current_swu_capacity = swu_capacity;
  }

  inline double SwuCapacity() const { return swu_capacity; }

  inline double CurrentSwuCapacity() const { return current_swu_capacity; }

  /// @brief this facility's initial conditions
  inline void  InitialReserves(double qty) { initial_reserves = qty; }
  inline double InitialReserves() const { return initial_reserves; }

 private:
  ///   @brief adds a material into the natural uranium inventory
  ///   @throws if the material is not the same composition as the in_recipe
  void AddMat_(cyclus::Material::Ptr mat);

  ///   @brief generates a request for this facility given its current state. The
  ///   quantity of the material will be equal to the remaining inventory size.
  cyclus::Material::Ptr Request_();

  ///  @brief Generates a material offer for a given request. The response
  ///  composition will be comprised only of U235 and U238 at their relative ratio
  ///  in the requested material. The response quantity will be the same as the
  ///  requested commodity.
  ///
  ///  @param req the requested material being responded to
  cyclus::Material::Ptr Offer_(cyclus::Material::Ptr req);

  cyclus::Material::Ptr Enrich_(cyclus::Material::Ptr mat, double qty);

  ///   @brief records and enrichment with the cyclus::Recorder
  void RecordEnrichment_(double natural_u, double swu);

  #pragma cyclus var {"tooltip": "input commodity", \
                      "doc": "commodity that the enrichment facility accepts", \
                      "uitype": "incommodity"}
  std::string in_commod;
  #pragma cyclus var {"tooltip": "output commodity", \
                      "doc": "commodity that the enrichment facility supplies", \
                      "uitype": "outcommodity"}
  std::string out_commod;
  #pragma cyclus var {"tooltip": "input commodity recipe", \
                      "doc": "recipe for enrichment facility's input commodity", \
                      "uitype": "recipe"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0.03, "tooltip": "tails assay", \
                      "doc": "tails assay from the enrichment process"}
  double tails_assay;
  #pragma cyclus var {"default": 1e299, "tooltip": "SWU capacity", \
                      "doc": "separative work unit (SWU) capcity of " \
                             "enrichment facility"}
  double swu_capacity;
  #pragma cyclus var {"default": 1e299, "tooltip": "maximum inventory size", \
                      "doc": "maximum inventory capacity of natural uranium in " \
                             "the enrichment facility"}
  double max_inv_size;
  #pragma cyclus var {"default": 0, "tooltip": "initial uranium reserves", \
                      "doc": "amount of natural uranium stored at the " \
                             "enrichment facility at the beginning of the " \
                             "simulation"}
  double initial_reserves;
  #pragma cyclus var {'derived_init': 'current_swu_capacity = swu_capacity;'}
  double current_swu_capacity;
  #pragma cyclus var {\
    'derived_init': "cyclus::Material::Ptr feed = "\
    "cyclus::Material::CreateUntracked(0, context()->GetRecipe(in_recipe)); "\
    "feed_assay = cyclus::toolkit::UraniumAssay(feed);", \
                      "tooltip": "feed assay", \
                      "doc": "feed assay for the enrichment process"}
  double feed_assay;
  #pragma cyclus var {'capacity': 'max_inv_size'}
  cyclus::toolkit::ResourceBuff inventory;  // of natl u

  friend class EnrichmentFacilityTest;
  // ---
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_ENRICHMENT_FACILITY_H_
