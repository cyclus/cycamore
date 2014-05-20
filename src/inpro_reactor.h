// inpro_reactor.h
#ifndef CYCAMORE_INPROREACTOR_INPRO_REACTOR_H_
#define CYCAMORE_INPROREACTOR_INPRO_REACTOR_H_

#include <map>
#include <string>

#include "bid_portfolio.h"
#include "capacity_constraint.h"
#include "commodity_producer.h"
#include "exchange_context.h"
#include "facility.h"
#include "facility.h"
#include "material.h"
#include "request_portfolio.h"
#include "resource_buff.h"

namespace cycamore {

class Context;
  
/**
   Defines all possible phases this facility can be in
 */
enum Phase {INIT, BEGIN, OPERATION, REFUEL, REFUEL_DELAY, WAITING, END};

/**
   @class InproReactor
   This class is identical to the RecipeReactor, except that it
   operates in a batch-like manner, i.e. it refuels in batches.
 */
class InproReactor : public cyclus::Facility,
  public cyclus::CommodityProducer {
 public:
  /* --- Module Members --- */
  /**
     Constructor for the InproReactor class.
     @param ctx the cyclus context for access to simulation-wide parameters
  */
  InproReactor(cyclus::Context* ctx);

  /**
     Destructor for the InproReactor class.
  */
  virtual ~InproReactor();

  virtual cyclus::Agent* Clone();

  /**
     initialize members from a different agent
  */
  void InitFrom(InproReactor* m);

  virtual std::string schema();

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::InfileTree object containing initialization data
  */
  virtual void InitFrom(cyclus::InfileTree* qe);

  /**
     Print information about this agent
  */
  virtual std::string str();
  /* --- */

  /* --- Facility Members --- */
  /**
     perform module-specific tasks when entering the simulation
   */
  virtual void Build(cyclus::Agent* parent);
  /* --- */

  /* --- Agent Members --- */  
  /// The Tick function specific to the BatchReactor.
  /// @param time the time of the tick
  virtual void Tick(int time);
  
  /// The Tick function specific to the BatchReactor.
  /// @param time the time of the tock
  virtual void Tock(int time);
  
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
  /* --- */

  /* --- InproReactor Members --- */
  /**
     return true if the cycle timer is >= the
     cycle length
   */
  bool CycleComplete();

  /**
     return true if the core is filled
   */
  bool CoreFilled();

  /**
     set the cycle length
     @param time the cycle length time
   */
  void set_cycle_length(int time);

  /**
     @return the cycle length
   */
  int cycle_length();

  /**
     set the time required to refuel the reactor
     @param time the refuel delay time
   */
  void set_refuel_delay(int time);

  /**
     @return the refuel delay time
   */
  int refuel_delay();

  /**
     set the input core loading
     @param size the core loading size in kilograms
   */
  void set_in_core_loading(double size);

  /**
     @return the input core loading in kilograms
   */
  double in_core_loading();

  /**
     set the output core loading
     @param size the core loading size out kilograms
   */
  void set_out_core_loading(double size);

  /**
     @return the output core loading out kilograms
   */
  double out_core_loading();

  /**
     set the number of batches per core
     @param n the number of batches to set
   */
  void set_batches_per_core(int n);

  /**
     @return the number of batches per core
   */
  int batches_per_core();

  /**
     return the batch loading
   */
  double BatchLoading();

  /**
     set the input commodity
     @param name the commodity name
   */
  void set_in_commodity(std::string name);

  /**
     @return the input commodity
  */
  std::string in_commodity();

  /**
     set the input recipe
     @param name the recipe name
   */
  void set_in_recipe(std::string name);

  /**
     @return the input recipe
  */
  std::string in_recipe();

  /**
     set the output commodity
     @param name the commodity name
   */
  void set_out_commodity(std::string name);

  /**
     @return the output commodity
   */
  std::string out_commodity();

  /**
     set the output recipe
     @param name the recipe name
   */
  void set_out_recipe(std::string name);

  /**
     @return the output recipe
   */
  std::string out_recipe();

  /**
     @return the current phase
  */
  Phase phase();

  /**
     set the next phase
     @param p the next phase
   */
  void SetPhase(Phase p);
  /* --- */

 protected:
  /* --- Facility Members --- */
  /**
     facilities over write this method if a condition must be met
     before their destructors can be called
  */
  virtual bool CheckDecommissionCondition();
  
  /// @brief construct a request portfolio for an order of a given size
  cyclus::RequestPortfolio<cyclus::Material>::Ptr GetOrder_(double size);

  /// @brief gets bids for a commodity from a buffer
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids_(
      const cyclus::CommodMap<cyclus::Material>::type& commod_requests,
      std::string commod,
      cyclus::toolkit::ResourceBuff* buffer);
  
  /// @brief returns a qty of material from the a buffer
  cyclus::Material::Ptr TradeResponse_(
      double qty,
      cyclus::toolkit::ResourceBuff* buffer);
  /* --- */

 private:
  /* --- InproReactor Members and Members --- */
  /// a map of phase names
  static std::map<Phase, std::string> phase_names_;

  cyclus::Material::Ptr staged_precore_;

  /// The time between batch reloadings.
  int cycle_length_;

  /// The time required to refuel the reactor
  int refuel_delay_;

  /// The time the present reactor has spent in phase REFUEL_DELAY
  int time_delayed_;

  /// batches per core
  int batches_per_core_;

  /// The total mass per core upon entry
  double in_core_loading_;

  /// The total mass per core upon exit
  double out_core_loading_;

  /// the name of the input commodity
  std::string in_commodity_;

  /// the name of the input recipe
  std::string in_recipe_;

  /// the name of the output commodity
  std::string out_commodity_;

  /// the name of the output recipe
  std::string out_recipe_;

  /// The current time step in the cycle
  int cycle_timer_;

  /// The current phase this facility is in
  Phase phase_;

  /// a matbuff for material before they enter the core
  cyclus::toolkit::ResourceBuff preCore_;

  /// a matbuff for material while they are inside the core
  cyclus::toolkit::ResourceBuff inCore_;

  /// a matbuff for material after they exit the core
  cyclus::toolkit::ResourceBuff postCore_;

  /**
     populate the phase name map
   */
  void SetUpPhaseNames();

  /**
     resets the cycle timer
   */
  void reset_cycle_timer();

  /**
     load fuel from preCore_ into inCore_
   */
  void LoadCore();

  /**
     move a batch from inCore_ to postCore_
   */
  void OffloadBatch();

  /**
     move all material from inCore_ to postCore_
   */
  void OffloadCore();
  /* --- */
};
} // namespace cycamore
#endif
