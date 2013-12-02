// batch_reactor.h
#ifndef _BATCHREACTOR_H
#define _BATCHREACTOR_H

#include "facility_model.h"
#include "commodity_producer.h"

#include "resource_buff.h"

#include <string>
#include <queue>
#include <map>

namespace cycamore {

class Context;
  
/**
   Defines all possible phases this facility can be in
 */
enum Phase {INIT, BEGIN, OPERATION, REFUEL, REFUEL_DELAY, WAITING, END};

/* /\** */
/*    information about an entry into the spent fuel pool */
/*  *\/ */
/* struct PoolEntry */
/* { */
/*   int exit_time; */
/*   cyclus::Material::Ptr mat; */

/* PoolEntry(int time, mat_rsr_ptr mat) : exit_time(time), mat(mat_ptr) {}; */
/* }; */

/**
   @class BatchReactor
   This class is identical to the RecipeReactor, except that it
   operates in a batch-like manner, i.e. it refuels in batches.
 */
class BatchReactor : public cyclus::FacilityModel,
  public cyclus::CommodityProducer {
 public:
  /* --- Module Methods --- */
  /**
     Constructor for the BatchReactor class.
     @param ctx the cyclus context for access to simulation-wide parameters
  */
  BatchReactor(cyclus::Context* ctx);

  /**
     Destructor for the BatchReactor class.
  */
  virtual ~BatchReactor();

  virtual cyclus::Model* Clone();

  virtual std::string schema();

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
     perform module-specific tasks when entering the simulation
   */
  virtual void Deploy(cyclus::Model* parent);
  /* --- */

  /* --- Agent Methods --- */
  /**
     The HandleTick function specific to the BatchReactor.
     @param time the time of the tick
   */
  virtual void HandleTick(int time);

  /**
     The HandleTick function specific to the BatchReactor.
     @param time the time of the tock
   */
  virtual void HandleTock(int time);
  /* --- */

  /* --- cyclus::Transaction Methods --- */
  /**
     When the facility receives a message, execute any transaction
   */
  virtual void ReceiveMessage(cyclus::Message::Ptr msg);

  /**
     send messages up through the institution
     @param recipient the final recipient
     @param trans the transaction to send
   */
  void SendMessage(Communicator* recipient, cyclus::Transaction trans);

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

  /* --- BatchReactor Methods --- */
  /**
     set the cycle length
     @param time the cycle length time
   */
  void cycle_length(int time);

  /**
     @return the cycle length
   */
  int cycle_length();

  /**
     set the time required to refuel the reactor
     @param time the refuel delay time
   */
  void refuel_delay(int time);

  /**
     @return the refuel delay time
   */
  int refuel_delay();

  /**
     set the input core loading
     @param size the core loading size in kilograms
   */
  void in_core_loading(double size);

  /**
     @return the input core loading in kilograms
   */
  double in_core_loading();

  /**
     set the output core loading
     @param size the core loading size out kilograms
   */
  void out_core_loading(double size);

  /**
     @return the output core loading out kilograms
   */
  double out_core_loading();

  /**
     set the number of batches per core
     @param n the number of batches to set
   */
  void batches_per_core(int n);

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
  void in_commodity(std::string name);

  /**
     @return the input commodity
  */
  std::string in_commodity();

  /**
     set the input recipe
     @param name the recipe name
   */
  void in_recipe(std::string name);

  /**
     @return the input recipe
  */
  std::string in_recipe();

  /**
     set the output commodity
     @param name the commodity name
   */
  void out_commodity(std::string name);

  /**
     @return the output commodity
   */
  std::string out_commodity();

  /**
     set the output recipe
     @param name the recipe name
   */
  void out_recipe(std::string name);

  /**
     @return the output recipe
   */
  std::string out_recipe();

  /**
     @return the current phase
  */
  Phase phase();
  /* --- */

 protected:
  /* --- Facility Methods --- */
  /**
     facilities over write this method if a condition must be met
     before their destructors can be called
  */
  virtual bool CheckDecommissionCondition();
  /* --- */

 private:
  /* --- BatchReactor Members and Methods --- */
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
  cyclus::ResourceBuff preCore_;

  /// a matbuff for material while they are inside the core
  cyclus::ResourceBuff inCore_;

  /// a matbuff for material after they exit the core
  cyclus::ResourceBuff postCore_;

  /// The list of orders to process on the Tock
  std::deque<cyclus::Message::Ptr> ordersWaiting_;

  /**
     populate the phase name map
   */
  void SetUpPhaseNames();

  /**
     resets the cycle timer
   */
  void recycle_timer();

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
     set the next phase
     @param p the next phase
   */
  void SetPhase(Phase p);

  /**
     make reqest for a specific amount of fuel
   */
  void MakeRequest(double amt);

  /**
     offer all off-loaded fuel
   */
  void MakeOffers();

  /**
     sends a request of offer to the commodity's market
   */
  void interactWithMarket(std::string commod, double amt, cyclus::TransType type);

  /**
     Processes all orders in ordersWaiting_
   */
  void HandleOrders();

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
