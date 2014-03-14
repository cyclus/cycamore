// enrichment_facility.h
#ifndef CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_
#define CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

#include <string>

#include "cyclus.h"

namespace cyc = cyclus;

namespace cycamore {
  
/// @class SWUConverter
///
/// @brief The SWUConverter is a simple Converter class for material to
/// determine the amount of SWU required for their proposed enrichment
class SWUConverter : public cyc::Converter<cyc::Material> { 
 public:
  SWUConverter(double feed, double tails) : feed_(feed), tails_(tails) {}
  virtual ~SWUConverter() {}

  /// @brief provides a conversion for the SWU required
  inline virtual double convert(cyc::Material::Ptr m) {
    cyc::enrichment::Assays a(feed_,
                                 cyc::enrichment::UraniumAssay(m), tails_);
    return cyc::enrichment::SwuRequired(m->quantity(), a);
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
class NatUConverter : public cyc::Converter<cyc::Material> { 
 public:
  NatUConverter(double feed, double tails) : feed_(feed), tails_(tails) {}
  virtual ~NatUConverter() {}

  /// @brief provides a conversion for the amount of natural Uranium required
  inline virtual double convert(cyc::Material::Ptr m) {
    cyc::enrichment::Assays a(feed_,
                                 cyc::enrichment::UraniumAssay(m), tails_);
    return cyc::enrichment::FeedQty(m->quantity(), a);
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

/**
   @class EnrichmentFacility

   @section introduction Introduction
   The EnrichmentFacility is a simple Agent to model the enriching of natural
   Uranium in a Cyclus simulation. It requests its input recipe (nominally
   natural Uranium), and produces any amount of enriched Uranium, given the its
   natural uranium inventory constraint and its SWU capacity constraint.

   @section requests Requests   
   The EnrichmentFacility will request from the cyc::ResourceExchange a
   cyc::Material whose quantity is its remaining inventory capacity and whose
   composition is that of its input recipe.

   @section acctrade Accepting Trades
   The EnrichmentFacility adds any accepted trades to its inventory.
   
   @section bids Bids
   The EnrichmentFacility will bid on any request for its output commodity. It
   will bid either the request quantity, or the quanity associated with either
   its SWU constraint or natural uranium constraint, whichever is lower.

   @section extrades Executing Trades
   The EnrichmentFacility will execute trades for its output commodity in the
   following manner:
     #. Determine the trade's quantity and product assay
     #. Determine the natural Uranium and SWU requires to create that product
     #. Remove the required quantity of natural Uranium from its inventory
     #. Extract the appropriate composition of enriched Uranium
     #. Send the enriched Uranium as the trade resource
   
   @section gotchas Gotchas
   #. In its current form, the EnrichmentFacility can only accept
   cyc::Material having the composition of its input recipe. If a
   cyc::Material of a different composition is sent to it, an exception will
   be thrown.

   #. During the trading phase, an exception will be thrown if either the
   EnrichmentFacility's SWU or inventory constraint is breached.

   @section improvements Improvments   
   The primary improvement to the EnrichmentFacility would be to relax the
   requirement that all input material have the in_recipe composition (i.e.,
   allow different base enrichments of Uranium).

   How would I go about doing so? I'd likely develop an EnrichmentBuffer-type
   class that can be queried as to its SWU and natural Uranium capacity.
 */
class EnrichmentFacility : public cyc::FacilityModel {
 public:
  /* --- Module Members --- */
  /**
     Constructor for the EnrichmentFacility class
     @param ctx the cyclus context for access to simulation-wide parameters
   */
  EnrichmentFacility(cyc::Context* ctx);

  /**
     Destructor for the EnrichmentFacility class
   */
  virtual ~EnrichmentFacility();

  virtual std::string schema();

  virtual cyc::Model* Clone();

  virtual void InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di);

  virtual void InitFrom(cyc::QueryBackend* b);

  virtual void Snapshot(cyc::DbInit di);

  virtual void InitInv(cyc::Inventories& invs);

  virtual cyc::Inventories SnapshotInv();

  /**
     initialize members from a different model
  */
  void InitFrom(EnrichmentFacility* m);

  /**
     Print information about this model
   */
  virtual std::string str();
  /* --- */

  /* --- Facility Members --- */
  /// perform module-specific tasks when entering the simulation 
  virtual void Build(cyc::Model* parent);
  /* --- */
  
  /* --- Agent Members --- */
  /**
     Each facility is prompted to do its beginning-of-time-step
     stuff at the tick of the timer.

     @param time is the time to perform the tick
   */
  virtual void Tick(int time);

  /**
     Each facility is prompted to its end-of-time-step
     stuff on the tock of the timer.

     @param time is the time to perform the tock
   */
  virtual void Tock(int time);

  /// @brief The EnrichmentFacility request Materials of its given
  /// commodity. 
  virtual std::set<cyc::RequestPortfolio<cyc::Material>::Ptr>
      GetMatlRequests();

  /// @brief The EnrichmentFacility place accepted trade Materials in their
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyc::Trade<cyc::Material>,
      cyc::Material::Ptr> >& responses);
  
  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory or SWU capacity, it will
  /// offer its minimum of its capacities.
  virtual std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
      GetMatlBids(const cyc::CommodMap<cyc::Material>::type&
                  commod_requests);
  
  /// @brief respond to each trade with a material enriched to the appropriate
  /// level given this facility's inventory
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyc::Trade<cyc::Material> >& trades,
    std::vector<std::pair<cyc::Trade<cyc::Material>,
    cyc::Material::Ptr> >& responses);
  /* --- */

  /* --- EnrichmentFacility Members --- */
  /**
     @brief Determines if a particular material is a valid request to respond
     to.  Valid requests must contain U235 and U238 and must have a relative
     U235-to-U238 ratio less than this facility's tails_assay().
     @return true if the above description is met by the material
  */
  bool ValidReq(const cyc::Material::Ptr mat);

  inline void in_commodity(std::string in_commod) { in_commod_ = in_commod; }

  inline std::string in_commodity() const { return in_commod_; }

  inline void out_commodity(std::string out_commod) { 
    out_commod_ = out_commod;
  }

  inline std::string out_commodity() const { return out_commod_; }

  inline void in_recipe(std::string in_recipe) { in_recipe_ = in_recipe; }

  inline std::string in_recipe() const { return in_recipe_; }

  inline void SetMaxInventorySize(double size) {
    inventory_.set_capacity(size);
  }

  inline double MaxInventorySize() const { return inventory_.capacity(); }

  inline double InventorySize() const { return inventory_.quantity(); }

  inline void feed_assay(double assay) { feed_assay_ = assay; }

  inline double feed_assay() const { return feed_assay_; }

  inline void tails_assay(double assay) { tails_assay_ = assay; }

  inline double tails_assay() const { return tails_assay_; }

  inline void swu_capacity(double capacity) {
    swu_capacity_ = capacity;
    current_swu_capacity_ = swu_capacity_;
  }

  inline double swu_capacity() const { return swu_capacity_; }

  inline double current_swu_capacity() const { return current_swu_capacity_; }

  /// @brief this facility's initial conditions
  inline void  initial_reserves(double qty) { initial_reserves_ = qty; }
  inline double initial_reserves() const { return initial_reserves_; }

 private:
  /**
     @brief adds a material into the natural uranium inventory
     @throws if the material is not the same composition as the in_recipe
   */
  void AddMat_(cyc::Material::Ptr mat);

  /**
     @brief generates a request for this facility given its current state. The
     quantity of the material will be equal to the remaining inventory size.
   */
  cyc::Material::Ptr Request_();
  
  /**
     @brief Generates a material offer for a given request. The response
     composition will be comprised only of U235 and U238 at their relative ratio
     in the requested material. The response quantity will be the same as the
     requested commodity.

     @param req the requested material being responded to
   */
  cyc::Material::Ptr Offer_(cyc::Material::Ptr req);

  /**
   */
  cyc::Material::Ptr Enrich_(cyc::Material::Ptr mat, double qty);

  /**
     @brief records and enrichment with the cyc::Recorder
   */
  void RecordEnrichment_(double natural_u, double swu);


  std::string in_commod_;
  std::string out_commod_;
  std::string in_recipe_;
  double feed_assay_;
  double tails_assay_;
  double swu_capacity_;
  double current_swu_capacity_;
  double initial_reserves_;
  cyc::ResourceBuff inventory_; // of natl u
  
  friend class EnrichmentFacilityTest;
/* --- */
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

