// source_facility.h
#ifndef CYCAMORE_MODELS__SOURCE_FACILITY_H_
#define CYCAMORE_MODELS__SOURCE_FACILITY_H_

#include <set>
#include <vector>

#include "cyclus.h"

namespace cycamore {

class Context;

/**
   @class SourceFacility
   This cyclus::Facility provides a simple source of some capacity
   (possibly infinite) of some commodity/Recipe.

   The SourceFacility class inherits from the cyclus::Facility class and is
   dynamically loaded by the Agent class when requested.


   @section introduction Introduction
   The SourceFacility is a facility type in Cyclus capable of providing
   a finite or infinite.Supply of a particular material to the
   simulation.  A SourceFacility generates material of a certain
   composition and commodity type, then offers that material on the
   appropriate market. Shipments of this material are executed when the
   market issues an order that the offer has been matched with a
   request.
   @section agentparams Agent Parameters
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
   -  map<Nucs, NumDens> outComp

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
class SourceFacility : public cyclus::Facility,
  public cyclus::CommodityProducer {
 public:
  /* --- Module Members --- */
  /**
     Constructor for the SourceFacility class
     @param ctx the cyclus context for access to simulation-wide parameters
   */
  SourceFacility(cyclus::Context* ctx);

  virtual ~SourceFacility();

  #pragma cyclus decl

  /**
     Print information about this agent
   */
  virtual std::string str();
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
  
  /// @brief Responds to each request for this source facility's commodity.
  /// If a given request is more than this facility's capacity, it will offer
  /// its capacity.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(const cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);
  
  /// @brief respond to each trade with a material made from this facility's
  /// recipe
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);
  /* --- */

  /* --- SourceFacility Members --- */
  /**
     @brief creates a material object to offer to a requester
     @param target the material target a request desires
   */
  cyclus::Material::Ptr GetOffer(const cyclus::Material::Ptr target) const;

  /**
     sets the output commodity name
     @param name the commodity name
   */
  inline void commodity(std::string name) { out_commod_ = name; }

  /// @return the output commodity
  inline std::string commodity() const { return out_commod_; }

  /**
     sets the capacity of a material generated at any given time step
     @param capacity the production capacity
   */
  inline void capacity(double capacity) {
    capacity_ = capacity;
    current_capacity_ = capacity_;
  }

  /// @return the production capacity at any given time step
  inline double capacity() const { return capacity_; }

  /**
     sets the name of the recipe to be produced
     @param name the recipe name
   */
  inline void recipe(std::string name) { recipe_name_ = name; }

  /// @return the name of the output recipe
  inline std::string recipe() const { return recipe_name_; }

  /// @return the current timestep's capacity
  inline double current_capacity() const { return current_capacity_; }

 private:
  /**
     This facility has only one output commodity
   */
  #pragma cyclus var {}
  std::string out_commod_;

  /**
     Name of the recipe this facility uses.
   */
  #pragma cyclus var {}
  std::string recipe_name_;

  /**
     The capacity is defined in terms of the number of units of the
     recipe that can be provided each time step.  A very large number
     can be provided to represent infinte capacity.
   */
  #pragma cyclus var {"default": 1e299}
  double capacity_;

  /**
     The capacity at the current time step
   */
  #pragma cyclus var {'derived_init': 'current_capacity_ = capacity_;'}
  double current_capacity_;

  /* --- */
};

} // namespace cycamore

#endif // CYCAMORE_MODELS__SOURCE_FACILITY_H_

