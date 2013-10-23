// sink_facility.h
#ifndef _SINKFACILITY_H
#define _SINKFACILITY_H

#include <string>

#include "facility_model.h"

#include "query_engine.h"
#include "resource_buff.h"
#include "logger.h"

namespace cycamore {

class Context;
  
/**
   @class SinkFacility
   This cyclus::FacilityModel requests a finite amount of its input commodity.
   It offers nothing.

   The SinkFacility class inherits from the cyclus::FacilityModel class and is
   dynamically loaded by the Model class when requested.

   @section intro Introduction
   The SinkFacility is a facility type in *Cyclus* capable of accepting
   a finite or infinite quantity of some commodity produced in the
   simulation. A SinkFacility requests an amount of that commodity from
   the appropriate market. It then receives that commodity when the
   market issues an order that the request has been matched with a
   corresponding offer.
   @section modelparams Model Parameters
   SinkFacility behavior is comprehensively defined by the following
   parameters:
   - double capacity: The acceptance capacity of the facility (units
   vary, but typically kg/month). Capacity is infinite if a positive
   value is provided.
   - int startDate: The date on which the facility begins to operate
   (months). - int lifeTime: The length of time that the facility
   operates (months). - std::string inCommod: The commodity type this
   facility accepts.
   @section optionalparams Optional Parameters
   SinkFacility behavior may also be specified with the following
   optional parameters which have default values listed here.
   - double capacityFactor: The ratio of actual acceptance capacity to
   the rated acceptance capacity. Default is 1 (actual/rated).
   - double availFactor: The percent of time the facility operates at
   its capacity factor. Default is 100%.
   - double capitalCost: The cost of constructing and commissioning this
   facility. Default is 0 ($).
   - double opCost: The annual cost of operation and maintenance of this
   facility. Default is 0 ($/year).
   - int constrTime: The number of months it takes to construct and
   commission this facility. Default is 0 (months).
   - int decomTime: The number of months it takes to deconstruct and
   decommission this facility. Default is 0 (months).
   - Inst* inst: The institution responsible for this facility.
   - string name: A non-generic name for this facility.

   @section detailed Detailed Behavior
   @subsection finite If Finite Capacity:
   The SinkFacility starts operation when the simulation reaches the
   month specified as the startDate. It immediately begins to request
   the inCommod commodity type at the rate defined by the SinkFacility
   capacity. If a request is matched with an offer from another
   facility, the SinkFacility executes that order by adding that
   quantity to its stocks. When the simulation time equals the startDate
   plus the lifeTime, the facility ceases to operate.

   @subsection infinite If Infinite Capacity:
   The SinkFacility starts operation when the simulation reaches the
   month specified as the startDate. Each month the SinkFacility
   requests an infinite amount of the inCommod commodity from the
   appropriate market. If there is a corresponding offer for that
   commodity type from another facility, the SinkFacility executes that
   order by adding that quantity to its stocks. When the simulation time
   equals the startDate plus the lifeTime, the facility ceases to
   operate.
   @subsection question Question:
   What is the best way to allow requests of an infinite amount of
   material on a market?
 */
class SinkFacility : public cyclus::FacilityModel  {
 public:
  /* --- Module Methods --- */
  /**
     Constructor for the SinkFacility class.
     @param ctx the cyclus context for access to simulation-wide parameters
  */
  SinkFacility(cyclus::Context* ctx);

  /**
     Destructor for the SinkFacility class.
  */
  virtual ~SinkFacility();

  virtual std::string schema();

  virtual cyclus::Model* Clone();

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitModuleMembers(cyclus::QueryEngine* qe);

  /**
     A verbose printer for the Sink Facility.
   */
  virtual std::string str();
  /* --- */

  /* --- Agent Methods --- */
  /**
     The SinkFacility can handle the Tick.

     @param time the current simulation time.
   */
  virtual void HandleTick(int time);

  /**
     The SinkFacility can handle the Tock.

     @param time the current simulation time.
   */
  virtual void HandleTock(int time);
  /* --- */

  /* --- cyclus::Transaction Methods --- */
  /**
     Transacted resources are received through this method

     @param trans the transaction to which these resource objects belong
     @param manifest is the set of resources being received
   */
  virtual void AddResource(cyclus::Transaction trans,
                           std::vector<cyclus::Resource::Ptr> manifest);

  /**
     The sink Facility doesn't need to do anything if it gets a message.
     It never sends any matieral to anyone.
   */
  virtual void ReceiveMessage(cyclus::Message::Ptr msg) {};
  /* --- */

  /* --- SinkFacility Methods --- */
  /**
     add a commodity to the set of input commodities
     @param name the commodity name
   */
  void AddCommodity(std::string name);

  /**
     sets the capacity of a material generated at any given time step
     @param capacity the reception capacity
   */
  void SetCapacity(double capacity);

  /// @return the reception capacity at any given time step
  double capacity();

  /**
     sets the size of the storage inventory for received material
     @param size the storage size
   */
  void SetMaxInventorySize(double size);

  /// @return the maximum inventory storage size
  double MaxInventorySize();

  /// @return the current inventory storage size
  double InventorySize();

  /// @return the input commodities
  std::vector<std::string> InputCommodities();
  /* --- */

 protected:
  /* --- SourceFacility Members and Methods --- */
  /**
     all facilities must have at least one input commodity
   */
  std::vector<std::string> in_commods_;

  /**
     monthly acceptance capacity
   */
  double capacity_;

  /**
     commodity price
   */
  double commod_price_;

  /**
     this facility holds material in storage.
   */
  cyclus::ResourceBuff inventory_;

  /**
     determines the amount to request
   */
  const double getRequestAmt();
};

} // namespace cycamore
#endif

