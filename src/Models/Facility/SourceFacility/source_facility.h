// source_facility.h
#ifndef _SOURCEFACILITY_H
#define _SOURCEFACILITY_H

#include "facility_model.h"
#include "query_engine.h"
#include "commodity_producer.h"

#include "mat_buff.h"

#include <deque>

/**
   @class SourceFacility
   This cyclus::FacilityModel provides a simple source of some capacity
   (possibly infinite) of some commodity/Recipe.

   The SourceFacility class inherits from the cyclus::FacilityModel class and is
   dynamically loaded by the Model class when requested.


   @section introduction Introduction
   The SourceFacility is a facility type in Cyclus capable of providing
   a finite or infinite.Supply of a particular material to the
   simulation.  A SourceFacility generates material of a certain
   composition and commodity type, then offers that material on the
   appropriate market. Shipments of this material are executed when the
   market issues an order that the offer has been matched with a
   request.
   @section modelparams Model Parameters
   SourceFacility behavior is comprehensively defined by the following
   parameters:
   -  double capacity: The production capacity of the facility (units
   vary, but typically kg/month). Capacity is infinite if a negative
   value is provided.
   -  int startDate: The date on which the facility begins to operate
   (months).
   -  int lifeTime: The length of time that the facility operates
   (months). -  std::string outCommod: the commodity that this facility
   produces -  double inventorysize: the maximum quantity of material to
   be held in the inventory
   -  double commodprice: the price of the output material PER UNIT
   -  map<Isos, NumDens> outComp

   @section optionalparams Optional Parameters
   SourceFacility behavior may also be specified with the following
   optional parameters which have default values listed here.
   -  double capacityFactor: The ratio of actual production capacity to
   the rated production capacity. Default is 1 (actual/rated).
   -  double availFactor: The percent of time the facility operates at
   its capacity factor. Default is 100%.
   -  double capitalCost: The cost of constructing and commissioning
   this facility. Default is 0 ($).
   -  double opCost: The annual cost of operation and maintenance of
   this facility. Default is 0 ( $/year).
   -  int constrTime: The number of months it takes to construct and
   commission this facility. Default is 0 (months).
   -  int decomTime: The number of months it takes to deconstruct and
   decommission this facility. Default is 0 (months).
   -  Inst* inst: The institution responsible for this facility.
   -  string name: A non-generic name for this facility.

   @section detailed Detailed Behavior
   @subsection finite If Finite Capacity:
   The SourceFacility starts operation when the simulation reaches the
   month specified as the startDate. It immediately begins to produce
   material at the rate defined by its capacity. Each month the
   SourceFacility adds the amount it has produced to its inventory. It
   then offers to the appropriate market exactly as much material as it
   has in its inventory. If an offer is matched with a request, the
   SourceFacility executes that order by subtracting the quantity from
   its inventory and sending that amount to the requesting facility.
   When the simulation time equals the startDate plus the lifeTime, the
   facility ceases to operate.
   @subsection infinite If Infinite Capacity:
   The SourceFacility starts operation when the simulation reaches the
   month specified as the startDate. Each month the SourceFacility
   offers an infinite amount of material to the appropriate market. If
   there is a request for that material, the SourceFacility executes
   that order by sending that amount to the requesting facility. When
   the simulation time equals the startDate plus the lifeTime, the
   facility ceases to operate.
   @subsection question Question:
   What is the best way to allow offers of an infinite amount of
   material on a market?
 */

class SourceFacility : public cyclus::FacilityModel,
  public cyclus::supply_demand::CommodityProducer {
 public:
  /* --- Module Methods --- */
  /**
     Default Constructor for the SourceFacility class
   */
  SourceFacility();

  /**
     Destructor for the SourceFacility class
   */
  virtual ~SourceFacility();

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

  /* --- Facility Methods --- */
  /**
     Copy module members from a source model
     @param sourceModel the model to copy from
   */
  virtual void CloneModuleMembersFrom(cyclus::FacilityModel* sourceModel);
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
  /* --- */

  /* --- SourceFacility Methods --- */
  /**
     sets the output commodity name
     @param name the commodity name
   */
  void SetCommodity(std::string name);

  /// @return the output commodity
  std::string commodity();

  /**
     sets the capacity of a material generated at any given time step
     @param capacity the production capacity
   */
  void SetCapacity(double capacity);

  /// @return the production capacity at any given time step
  double capacity();

  /**
     sets the name of the recipe to be produced
     @param name the recipe name
   */
  void setRecipe(std::string name);

  /// @return the name of the output recipe
  std::string recipe();

  /**
     sets the size of the storage inventory for produced material
     @param size the storage size
   */
  void setMaxInventorySize(double size);

  /// @return the maximum inventory storage size
  double maxInventorySize();

  /// @return the current inventory storage size
  double inventorySize();
  /* --- */

 protected:
  /* --- cyclus::Transaction Methods --- */
  /**
     builds a transaction
   */
  cyclus::Transaction buildTransaction();

  /**
     sends a transaction as an offer
   */
  void sendOffer(cyclus::Transaction trans);
  /* --- */

  /* --- SourceFacility Members and Methods --- */
  /**
     This facility has only one output commodity
   */
  std::string out_commod_;

  /**
     Name of the recipe this facility uses.
   */
  std::string recipe_name_;

  /**
     The capacity is defined in terms of the number of units of the
     recipe that can be provided each time step.  A very large number
     can be provided to represent infinte capacity.
   */
  double capacity_;

  /**
     The price that the facility will charge for its output commodity.
     Units vary and are in dollars per inventory unit.
   */
  double commod_price_;

  /**
     A collection  that holds the "product" cyclus::Material this Facility has
     on hand to send to others.
   */
  cyclus::MatBuff inventory_; // @MJG couldnt this be a RsrcBuff?

  /**
     A list of orders to be processed on the Tock
   */
  std::deque<cyclus::Message::Ptr> ordersWaiting_;

  /**
     generates a material
   */
  void generateMaterial();
  /* --- */
};

#endif

