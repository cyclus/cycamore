// storage_facility.h
#ifndef _STORAGEFACILITY_H
#define _STORAGEFACILITY_H

#include "facility_model.h"

#include "resource_buff.h"

#include <queue>

// forward declarations
class cyclus::QueryEngine;

/**
   @class StorageFacility
   This cyclus::FacilityModel stores material.

   The StorageFacility class inherits from the cyclus::FacilityModel class and
   is dynamically loaded by the Model class when requested.

   @section intro Introduction

   Place an introduction to the model here.

   @section modelparams Model Parameters

   Place a description of the required input parameters which define the
   model implementation.

   @section optionalparams Optional Parameters

   Place a description of the optional input parameters to define the
   model implementation.

   @section detailed Detailed Behavior

   Place a description of the detailed behavior of the model. Consider
   describing the behavior at the tick and tock as well as the behavior
   upon sending and receiving materials and messages.

 */
class StorageFacility : public cyclus::FacilityModel {
 public:
  /* --- Module Methods --- */
  /**
     Default constructor for the StorageFacility class.
   */
  StorageFacility() {};

  /**
     Destructor for the StorageFacility class.
   */
  virtual ~StorageFacility() {};

  /**
     Print information about this model
   */
  virtual std::string str();
  /* --- */

  /* --- Facility Methods --- */
  /**
     Copy module members from a source model
     @param sourceModel the model to copy from
   */
  virtual void CloneModuleMembersFrom(cyclus::FacilityModel* sourceModel);
  /* --- */

  /* --- Agent Methods --- */
  /**
     The HandleTick function specific to the StorageFacility.
     At each tick, it requests as much raw inCommod as it can process
     this month and offers as much outCommod as it will have in its
     inventory by the end of the month.
     @param time the time of the tick
   */
  virtual void HandleTick(int time);

  /**
     The HandleTick function specific to the StorageFacility.
     At each tock, it processes material and handles orders, and records
     this month's actions.
     @param time the time of the tock
   */
  virtual void HandleTock(int time);
  /* --- */

  /* --- cyclus::Transaction Methods --- */

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

  /**
     When the facility receives a message, execute any transaction
   */
  virtual void ReceiveMessage(cyclus::Message::Ptr msg);

 protected:
  /**
     builds a transaction
   */
  cyclus::Transaction buildTransaction();

  /**
     sends a transaction as an offer
   */
  void sendOffer(cyclus::Transaction trans);
  /* --- */

  /* --- StorageFacility Methods --- */
  /**
     sets the inputput commodity name
     @param name the commodity name
   */
  void setIncyclus::Commodity(std::string name);

  /// @return the input commodity
  std::string incyclus::Commodity();

  /**
     sets the capacity of a able to enter at any given time step
     @param capacity the input capacity
   */
  void setInCapacity(double capacity);

  /// @return the input capacity at any given time step
  double inCapacity();

  /**
     sets the outputput commodity name
     @param name the commodity name
   */
  void setOutcyclus::Commodity(std::string name);

  /// @return the output commodity
  std::string outcyclus::Commodity();

  /**
     sets the capacity of a able to enter at any given time step
     @param capacity the output capacity
   */
  void setOutCapacity(double capacity);

  /// @return the output capacity at any given time step
  double outCapacity();

 protected:
  /**
   */
  std::string in_commod_;

  /**
   */
  std::string out_commod_;

  /**
     The minimum time that the stock material spends in the facility.
   */
  int residence_time_;

  /**
   */
  double offer_price_;

  /**
     The stocks of entering material
     These are not yet old enough to leave
   */
  ResourceBuff in_buffer_;

  /**
     The stocks of entering material
     These are not yet old enough to leave
   */
  ResourceBuff out_buffer_;

  /**
     The list of orders to process on the Tock
   */
  std::deque<cyclus::Message::Ptr> ordersWaiting_;

  /**
   */
  std::queue<double> enter_times_;
  /* ------------------- */

};

#endif

