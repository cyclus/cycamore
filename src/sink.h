#ifndef CYCAMORE_SRC_SINK_H_
#define CYCAMORE_SRC_SINK_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cyclus.h"

namespace cycamore {

class Context;

///   @class Sink
///   This cyclus::Facility requests a finite amount of its input commodity.
///  It offers nothing.
///
///  The Sink class inherits from the cyclus::Facility class and is
///  dynamically loaded by the Agent class when requested.
///
///  @section intro Introduction
///  The Sink is a facility type in *Cyclus* capable of accepting
///  a finite or infinite quantity of some commodity produced in the
///  simulation. A Sink requests an amount of that commodity from
///  the appropriate market. It then receives that commodity when the
///  market issues an order that the request has been matched with a
///  corresponding offer.
///  @section agentparams Agent Parameters
///  Sink behavior is comprehensively defined by the following
///  parameters:
///  - double capacity: The acceptance capacity of the facility (units
///  vary, but typically kg/month). Capacity is infinite if a positive
///  value is provided.
///  - int startDate: The date on which the facility begins to operate
///  (months). - int lifeTime: The length of time that the facility
///  operates (months). - std::string inCommod: The commodity type this
///  facility accepts.
///  @section optionalparams Optional Parameters
///  Sink behavior may also be specified with the following
///  optional parameters which have default values listed here.
///  - double capacityFactor: The ratio of actual acceptance capacity to
///  the rated acceptance capacity. Default is 1 (actual/rated).
///  - double AvailFactor: The percent of time the facility operates at
///  its capacity factor. Default is 100%.
///  - double capitalCost: The cost of constructing and commissioning this
///  facility. Default is 0 ($).
///  - double opCost: The annual cost of operation and maintenance of this
///  facility. Default is 0 ($/year).
///  - int constrTime: The number of months it takes to construct and
///  commission this facility. Default is 0 (months).
///  - int decomTime: The number of months it takes to deconstruct and
///  decommission this facility. Default is 0 (months).
///  - Inst* inst: The institution responsible for this facility.
///  - string name: A non-generic name for this facility.
///
///  @section detailed Detailed Behavior
///  @subsection finite If Finite Capacity:
///  The Sink starts operation when the simulation reaches the
///  month specified as the startDate. It immediately begins to request
///  the inCommod commodity type at the rate defined by the Sink
///  capacity. If a request is matched with an offer from another
///  facility, the Sink executes that order by adding that
///  quantity to its stocks. When the simulation time equals the startDate
///  plus the lifeTime, the facility ceases to operate.
///
///  @subsection infinite If Infinite Capacity:
///  The Sink starts operation when the simulation reaches the
///  month specified as the startDate. Each month the Sink
///  requests an infinite amount of the inCommod commodity from the
///  appropriate market. If there is a corresponding offer for that
///  commodity type from another facility, the Sink executes that
///  order by adding that quantity to its stocks. When the simulation time
///  equals the startDate plus the lifeTime, the facility ceases to
///  operate.
///  @subsection question Question:
///  What is the best way to allow requests of an infinite amount of
///  material on a market?
class Sink : public cyclus::Facility  {
 public:
  // --- Module Members ---
  ///  Constructor for the Sink class.
  ///  @param ctx the cyclus context for access to simulation-wide parameters
  Sink(cyclus::Context* ctx);

  ///   Destructor for the Sink class.
  virtual ~Sink();

  #pragma cyclus decl

  #pragma cyclus note {"doc": "A sink facility that accepts specified " \
                              "amounts of commodities from other agents"}

  ///   A verbose printer for the Sink Facility.
  virtual std::string str();
  // ---

  // --- Agent Members ---
  /// The Sink can handle the Tick.

  /// @param time the current simulation time.
  virtual void Tick();

  /// The Sink can handle the Tock.

  /// @param time the current simulation time.
  virtual void Tock();

  /// @brief SinkFacilities request Materials of their given commodity. Note
  /// that it is assumed the Sink operates on a single resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  /// @brief SinkFacilities request Products of their given
  /// commodity. Note that it is assumed the Sink operates on a single
  /// resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
      GetGenRsrcRequests();

  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptGenRsrcTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Product>,
      cyclus::Product::Ptr> >& responses);
  // ---

  // --- Sink Members ---
  ///  add a commodity to the set of input commodities
  ///  @param name the commodity name
  inline void AddCommodity(std::string name) { in_commods.push_back(name); }

  ///  sets the size of the storage inventory for received material
  ///  @param size the storage size
  inline void SetMaxInventorySize(double size) {
    max_inv_size = size;
    inventory.set_capacity(size);
  }

  /// @return the maximum inventory storage size
  inline double MaxInventorySize() const { return inventory.capacity(); }

  /// @return the current inventory storage size
  inline double InventorySize() const { return inventory.quantity(); }

  /// determines the amount to request
  inline double RequestAmt() const {
    return std::min(capacity, std::max(0.0, inventory.space()));
  }

  /// sets the capacity of a material generated at any given time step
  /// @param capacity the reception capacity
  inline void Capacity(double cap) { capacity = cap; }

  /// @return the reception capacity at any given time step
  inline double Capacity() const { return capacity; }

  /// @return the input commodities
  inline const std::vector<std::string>&
      input_commodities() const { return in_commods; }

 private:
  /// all facilities must have at least one input commodity
  #pragma cyclus var {"tooltip": "input commodities", \
                      "doc": "commodities that the sink facility accepts", \
                      "uitype": ["oneormore", "incommodity"]}
  std::vector<std::string> in_commods;

  /// monthly acceptance capacity
  #pragma cyclus var {"default": 1e299, "tooltip": "sink capacity", \
                      "doc": "capacity the sink facility can " \
                             "accept at each time step"}
  double capacity;

  /// max inventory size
  #pragma cyclus var {"default": 1e299, \
                      "tooltip": "sink maximum inventory size", \
                      "doc": "total maximum inventory size of sink facility"}
  double max_inv_size;

  /// this facility holds material in storage.
  #pragma cyclus var {'capacity': 'max_inv_size'}
  cyclus::toolkit::ResourceBuff inventory;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SINK_H_
