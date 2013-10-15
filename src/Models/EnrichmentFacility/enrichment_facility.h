// enrichment_facility.h
#ifndef _ENRICHMENTFACILITY_H
#define _ENRICHMENTFACILITY_H

#include "facility_model.h"

#include "mat_buff.h"
#include "transaction.h"
#include "enrichment.h"

#include <string>
#include <deque>

namespace cycamore {

class Context;

/**
   @class EnrichmentFacility

   @section introduction Introduction
 */
class EnrichmentFacility : public cyclus::FacilityModel {
 public:
  /* --- Module Methods --- */
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

  /* --- Agent Methods --- */
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

  /* --- cyclus::Transaction Methods --- */
  /**
     When this facility receives a message, execute the transaction
   */
  virtual void ReceiveMessage(cyclus::Message::Ptr msg);

  /**
     Transacted resources are extracted through this method
     @param order the msg/order for which resource(s) are to be prepared
     @return list of resources to be sent for this order
   */
  virtual std::vector<cyclus::Resource::Ptr> RemoveResource(
    cyclus::Transaction order);

  /**
     Transacted resources are received through this method
     @param trans the transaction to which these resource objects belong
     @param manifest is the set of resources being received
   */
  virtual void AddResource(cyclus::Transaction trans,
                           std::vector<cyclus::Resource::Ptr> manifest);
  /* --- */

  /* --- EnrichmentFacility Methods --- */
  cyclus::enrichment::Assays GetAssays(cyclus::Material::Ptr mat);

  inline void set_in_commodity(std::string in_commod) {
    in_commodity_ = in_commod;
  }

  inline std::string in_commodity() {
    return in_commodity_;
  }

  inline void set_out_commodity(std::string out_commod) {
    out_commodity_ = out_commod;
  }

  inline std::string out_commodity() {
    return out_commodity_;
  }

  inline void set_in_recipe(std::string in_recipe) {
    in_recipe_ = in_recipe;
  }

  inline std::string in_recipe() {
    return in_recipe_;
  }

  inline void SetMaxInventorySize(double size) {
    inventory_.SetCapacity(size);
  }

  inline double MaxInventorySize() {
    return inventory_.capacity();
  }

  inline double inventoryQty() {
    return inventory_.quantity();
  }

  inline void set_feed_assay(double assay) {
    feed_assay_ = assay;
  }

  inline double feed_assay() {
    return feed_assay_;
  }

  inline void set_tails_assay(double assay) {
    tails_assay_ = assay;
  }

  inline double tails_assay() {
    return tails_assay_;
  }

  inline void set_commodity_price(double price) {
    commodity_price_ = price;
  }

  inline double commodity_price() {
    return commodity_price_;
  }
  /* --- */

 protected:
  /* --- cyclus::Transaction Methods --- */
  /**
     sends a transaction as an offer
   */
  void SendOffer(cyclus::Transaction trans);

  /**
   */
  void MakeRequest();

  /**
   */
  void MakeOffer();

  /**
   */
  cyclus::Transaction BuildTransaction();
  /* --- */

  /* --- EnrichmentFacility Members and Methods --- */
  /**
   */
  void ProcessOutgoingMaterial();

  /**
   */
  void RecordEnrichment(double natural_u, double swu);

  std::string in_commodity_;

  std::string out_commodity_;

  std::string in_recipe_;

  double commodity_price_;

  double feed_assay_;

  double tails_assay_;

  cyclus::MatBuff inventory_;

  static int entry_;

  ///   A list of orders to be processed on the Tock
  std::deque<cyclus::Message::Ptr> orders_;
  /* --- */
};
} // namespace cycamore
#endif

