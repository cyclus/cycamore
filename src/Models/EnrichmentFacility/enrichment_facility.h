// enrichment_facility.h
#ifndef CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_
#define CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

#include <string>

#include "enrichment.h"
#include "facility_model.h"
#include "resource_buff.h"
#include "transaction.h"
#include "material.h"

// forward declarations
namespace cycamore {
class EnrichmentFacility;
} // namespace cycamore
namespace cyclus {  
class Context;
} // namespace cyclus

// forward includes
#include "enrichment_facility_tests.h"

namespace cycamore {

/**
   @class EnrichmentFacility

   @section introduction Introduction
   The EnrichmentFacility is a simple Agent to model the enriching of natural
   Uranium in a Cyclus simulation. It requests its input recipe (nominally
   natural Uranium), and produces any amount of enriched Uranium, given the its
   natural uranium inventory constraint and its SWU capacity constraint.

   @section requests Requests   
   The EnrichmentFacility will request from the cyclus::ResourceExchange a
   cyclus::Material whose quantity is its remaining inventory capacity and whose
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
   cyclus::Material having the composition of its input recipe. If a
   cyclus::Material of a different composition is sent to it, an exception will
   be thrown.

   #. During the trading phase, an exception will be thrown if either the
   EnrichmentFacility's SWU or inventory constraint is breached.
 */
class EnrichmentFacility : public cyclus::FacilityModel {
 public:
  /* --- Module Members --- */
  /**
     Constructor for the EnrichmentFacility class
     @param ctx the cyclus context for access to simulation-wide parameters
   */
  EnrichmentFacility(cyclus::Context* ctx);

  /**
     Destructor for the EnrichmentFacility class
   */
  virtual ~EnrichmentFacility();

  virtual std::string schema();

  virtual cyclus::Model* Clone();

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitModuleMembers(cyclus::QueryEngine* qe);

  /**
     Print information about this model
   */
  virtual std::string str();
  /* --- */

  /* --- Agent Members --- */
  /**
     Each facility is prompted to do its beginning-of-time-step
     stuff at the tick of the timer.

     @param time is the time to perform the tick
   */
  virtual void HandleTick(int time);

  /**
     Each facility is prompted to its end-of-time-step
     stuff on the tock of the timer.

     @param time is the time to perform the tock
   */
  virtual void HandleTock(int time);
  
  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory or SWU capacity, it will
  /// offer its minimum of its capacities.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      AddMatlBids(cyclus::ExchangeContext<cyclus::Material>* ec);
  
  /// /// @brief respond to each trade with a material enriched to the appropriate
  /// /// level given this facility's inventory
  /// ///
  /// /// @param trades all trades in which this trader is the supplier
  /// /// @param responses a container to populate with responses to each trade
  /// virtual void PopulateMatlTradeResponses(
  ///   const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  ///   std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  ///   cyclus::Material::Ptr> >& responses);
  /* --- */

  /* --- EnrichmentFacility Members --- */
  cyclus::enrichment::Assays GetAssays(cyclus::Material::Ptr mat);

  inline void in_commodity(std::string in_commod) { in_commodity_ = in_commod; }

  inline std::string in_commodity() const { return in_commodity_; }

  inline void out_commodity(std::string out_commod) { 
    out_commodity_ = out_commod;
  }

  inline std::string out_commodity() const { return out_commodity_; }

  inline void in_recipe(std::string in_recipe) { in_recipe_ = in_recipe; }

  inline std::string in_recipe() const { return in_recipe_; }

  inline void SetMaxInventorySize(double size) {
    inventory_.set_capacity(size);
  }

  inline double MaxInventorySize() const { return inventory_.capacity(); }

  inline double InventoryQty() const { return inventory_.quantity(); }

  inline void feed_assay(double assay) { feed_assay_ = assay; }

  inline double feed_assay() const { return feed_assay_; }

  inline void tails_assay(double assay) { tails_assay_ = assay; }

  inline double tails_assay() const { return tails_assay_; }

  inline void swu_capacity(double capacity) { swu_capacity_ = capacity; }

  inline double swu_capacity() const { return swu_capacity_; }

  inline void commodity_price(double price) { commodity_price_ = price; }

  inline double commodity_price() const { return commodity_price_; }

 private:
  /// /**
  ///    sends a transaction as an offer
  ///  */
  /// void SendOffer_(cyclus::Transaction trans);

  /// /**
  ///  */
  /// void MakeRequest();

  /// /**
  ///  */
  /// void MakeOffer();
  /* --- */

  /// /**
  ///  */
  /// void ProcessOutgoingMaterial_();

  /**
     @brief absorbs a material into the natural uranium inventory
     @throws if the material is not the same composition as the in_recipe
   */
  void Absorb_(cyclus::Material::Ptr mat);

  /**
     @brief records and enrichment with the cyclus::EventManager
   */
  void RecordEnrichment_(double natural_u, double swu);

  std::string in_commodity_;
  std::string out_commodity_;
  std::string in_recipe_;
  double commodity_price_;
  double feed_assay_;
  double tails_assay_;
  double swu_capacity_;
  cyclus::ResourceBuff inventory_; // of natl u
  static int entry_;
  /// cyclus::Converter converter_;
  friend class EnrichmentFacilityTest;
/* --- */
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

