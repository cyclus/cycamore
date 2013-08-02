// BatchReactor.h
#ifndef _BATCHREACTOR_H
#define _BATCHREACTOR_H

#include "FacilityModel.h"
#include "CommodityProducer.h"

#include "MatBuff.h"

#include <string>
#include <queue>
#include <map>

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
/*   cyclus::mat_rsrc_ptr mat; */
  
/* PoolEntry(int time, mat_rsr_ptr mat) : exit_time(time), mat(mat_ptr) {}; */
/* }; */

/**
   @class BatchReactor 
   This class is identical to the RecipeReactor, except that it
   operates in a batch-like manner, i.e. it refuels in batches.
 */
class BatchReactor : public cyclus::FacilityModel, public SupplyDemand::cyclus::CommodityProducer  
{
 public:  
  /* --- Module Methods --- */
  /**
     Constructor for the BatchReactor class. 
  */
  BatchReactor();
  
  /**
     Destructor for the BatchReactor class. 
  */
  virtual ~BatchReactor();
  
  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
  */
  virtual void initModuleMembers(cyclus::QueryEngine* qe);
  
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
  virtual void cloneModuleMembersFrom(cyclus::FacilityModel* sourceModel);

  /**
     perform module-specific tasks when entering the simulation
   */
  virtual void enterSimulationAsModule();
  /* --- */

  /* --- Agent Methods --- */
  /**
     The handleTick function specific to the BatchReactor.
     @param time the time of the tick
   */
  virtual void handleTick(int time);

  /**
     The handleTick function specific to the BatchReactor.
     @param time the time of the tock
   */
  virtual void handleTock(int time);
  /* --- */

  /* --- cyclus::Transaction Methods --- */
  /**
     When the facility receives a message, execute any transaction
   */
  virtual void receiveMessage(cyclus::msg_ptr msg);

  /**
     send messages up through the institution 
     @param recipient the final recipient 
     @param trans the transaction to send 
   */
  void sendMessage(Communicator* recipient, cyclus::Transaction trans);

  /**
     Transacted resources are extracted through this method
     @param order the msg/order for which resource(s) are to be prepared
     @return list of resources to be sent for this order
   */
  virtual std::vector<cyclus::rsrc_ptr> removeResource(cyclus::Transaction order);

  /**
     Transacted resources are received through this method      
     @param trans the transaction to which these resource objects belong
     @param manifest is the set of resources being received
   */
  virtual void addResource(cyclus::Transaction trans,
        		   std::vector<cyclus::rsrc_ptr> manifest);
  /* --- */

  /* --- BatchReactor Methods --- */
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
  double batchLoading();

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
  /* --- */

 protected:
  /* --- Facility Methods --- */
  /**
     facilities over write this method if a condition must be met
     before their destructors can be called
  */
  virtual bool checkDecommissionCondition();
  /* --- */

 private:
  /* --- BatchReactor Members and Methods --- */
  /// a map of phase names
  static std::map<Phase,std::string> phase_names_;

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
  cyclus::MatBuff preCore_;

  /// a matbuff for material while they are inside the core
  cyclus::MatBuff inCore_;

  /// a matbuff for material after they exit the core
  cyclus::MatBuff postCore_;

  /// The list of orders to process on the Tock 
  std::deque<cyclus::msg_ptr> ordersWaiting_;

  /**
     populate the phase name map
   */
  void setUpPhaseNames();

  /**
     resets the cycle timer
   */
  void reset_cycle_timer();

  /**
     return true if the cycle timer is >= the 
     cycle length
   */
  bool cycleComplete();

  /**
     return true if the core is filled
   */
  bool coreFilled();

  /**
     set the next phase
     @param p the next phase
   */
  void setPhase(Phase p);

  /**
     make reqest for a specific amount of fuel
   */
  void makeRequest(double amt);

  /**
     offer all off-loaded fuel
   */
  void makeOffers();

  /**
     sends a request of offer to the commodity's market
   */
  void interactWithMarket(std::string commod, double amt, TransType type);

  /**
     Processes all orders in ordersWaiting_
   */
  void handleOrders();

  /**
     move a certain amount of fuel from one buffer to another
     @param fromBuff the buffer to move fuel from
     @param toBuff the buffer to move fuel to
     @param amt the amount of fuel to move
  */
  void moveFuel(cyclus::MatBuff& fromBuff, cyclus::MatBuff& toBuff, double amt);

  /**
     moves and amount of fuel out of the core. this action will remove
     the amount of fuel from the core and add a different amount to
     the recieving buffer by a factor of 
     out_core_loading()/in_core_loading(). The recipe will also be 
     changed to out_recipe();
     @param amt the amount of fuel to offload
  */
  void offLoadFuel(double amt);

  /**
     load fuel from preCore_ into inCore_
   */
  void loadCore();

  /**
     move a batch from inCore_ to postCore_
   */
  void offloadBatch();

  /**
     move all material from inCore_ to postCore_
   */
  void offloadCore();
  /* --- */
};

#include "CycException.h"
/**
   An exception class for BatchReactors that aren't empty when
   their destructor is called
*/
class cyclus::CycBatchReactorDestructException : public cyclus::CycException {
 public: 
 cyclus::CycBatchReactorDestructException(std::string msg) : cyclus::CycException(msg) {};
};

/**
   An exception class for BatchReactors that exhibit undefined behavior
   for a given phase
*/
class cyclus::CycBatchReactorPhaseBehaviorException : public cyclus::CycException {
 public: 
 cyclus::CycBatchReactorPhaseBehaviorException(std::string msg) : cyclus::CycException(msg) {};
};

#endif
