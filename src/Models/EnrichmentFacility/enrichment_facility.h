// enrichment_facility.h
#ifndef CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_
#define CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

#include <string>

#include "enrichment.h"
#include "facility_model.h"
#include "resource_buff.h"
#include "transaction.h"

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
 */
class EnrichmentFacility : public cyclus::FacilityModel {
  friend class EnrichmentFacilityTest;
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

  inline void SetMaxInventorySize(double size) { inventory_.set_capacity(size); }

  inline double MaxInventorySize() const { return inventory_.capacity(); }

  inline double InventoryQty() const { return inventory_.quantity(); }

  inline void feed_assay(double assay) { feed_assay_ = assay; }

  inline double feed_assay() const { return feed_assay_; }

  inline void tails_assay(double assay) { tails_assay_ = assay; }

  inline double tails_assay() const { return tails_assay_; }

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
   */
  void RecordEnrichment_(double natural_u, double swu);

  std::string in_commodity_;
  std::string out_commodity_;
  std::string in_recipe_;
  double commodity_price_;
  double feed_assay_;
  double tails_assay_;
  cyclus::ResourceBuff inventory_;
  static int entry_;
  /* --- */
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_ENRICHMENTFACILITY_ENRICHMENT_FACILITY_H_

