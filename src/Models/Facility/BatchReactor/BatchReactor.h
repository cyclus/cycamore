// BatchReactor.h
#ifndef _BATCHREACTOR_H
#define _BATCHREACTOR_H

#include "FacilityModel.h"

#include "MatBuff.h"

#include <string>
#include <queue>

/**
   Defines all possible phases this facility can be in
 */
enum Phase {INIT, BEGIN, OPERATION, REFUEL, WAITING, END};

/**
   @class BatchReactor 
   This class is identical to the RecipeReactor, except that it
   operates in a batch-like manner, i.e. it refuels in batches.
 */

class BatchReactor : public FacilityModel  {
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
     @param qe a pointer to a QueryEngine object containing initialization data
  */
  virtual void initModuleMembers(QueryEngine* qe);
  
  /**
     Print information about this model 
  */
  virtual std::string str();
  /* --- */

  /* --- Facility Methods --- */
  /**
     prototypes are required to provide the capacity to copy their
     initialized members
   */
  virtual Prototype* clone();

  /**
     allows facilities to define what members need to be initialized
     after their prototypes have been cloned and entered into the 
     simulation
   */
  virtual void initializeConcreteMembers();
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

  /* --- Transaction Methods --- */
  /**
     When the facility receives a message, execute any transaction
   */
  virtual void receiveMessage(msg_ptr msg);

  /**
     send messages up through the institution 
     @param recipient the final recipient 
     @param trans the transaction to send 
   */
  void sendMessage(Communicator* recipient, Transaction trans);

  /**
     Transacted resources are extracted through this method
     @param order the msg/order for which resource(s) are to be prepared
     @return list of resources to be sent for this order
   */
  virtual std::vector<rsrc_ptr> removeResource(Transaction order);

  /**
     Transacted resources are received through this method      
     @param trans the transaction to which these resource objects belong
     @param manifest is the set of resources being received
   */
  virtual void addResource(Transaction trans,
        		   std::vector<rsrc_ptr> manifest);
  /* --- */

  /* --- BatchReactor Methods --- */
  /**
     set the cycle length 
     @param time the cycle length time
   */
  void setCycleLength(int time);

  /**
     @return the cycle length 
   */
  int cycleLength();

  /**
     set the core loading
     @param size the core loading size
   */
  void setCoreLoading(double size);

  /**
     @return the core loading
   */
  double coreLoading();

  /**
     set the number of batches per core
     @param n the number of batches to set
   */
  void setNBatches(int n);

  /**
     @return the number of batches per core
   */
  int nBatches();

  /**
     return the batch loading
   */
  double batchLoading();

  /**
     set the input commodity 
     @param name the commodity name
   */
  void setInCommodity(std::string name);

  /**
     @return the input commodity 
  */
  std::string inCommodity();

  /**
     set the input recipe 
     @param name the recipe name
   */
  void setInRecipe(std::string name);

  /**
     @return the input recipe 
  */
  std::string inRecipe();

  /**
     set the output commodity 
     @param name the commodity name
   */
  void setOutCommodity(std::string name);

  /**
     @return the output commodity
   */
  std::string outCommodity();

  /**
     set the output recipe 
     @param name the recipe name
   */
  void setOutRecipe(std::string name);

  /**
     @return the output recipe
   */
  std::string outRecipe();

  /**
     @return the current phase
  */
  Phase phase();
  /* --- */

 private:
  /* --- BatchReactor Members and Methods --- */
  /// The time between batch reloadings. 
  int cycle_length_;

  /// batches per core
  int batches_per_core_;
  
  /// The total mass per core
  double core_loading_;

  /// the name of the input commodity
  std::string in_commod_;

  /// the name of the input recipe
  std::string in_recipe_;

  /// the name of the output commodity
  std::string out_commod_;

  /// the name of the output recipe
  std::string out_recipe_;

  /// The current time step in the cycle
  int cycle_timer_;

  /// The current phase this facility is in
  Phase phase_;

  /// a matbuff for material before they enter the core
  MatBuff preCore_;

  /// a matbuff for material while they are inside the core
  MatBuff inCore_;

  /// a matbuff for material after they exit the core
  MatBuff postCore_;

  /// The list of orders to process on the Tock 
  std::deque<msg_ptr> ordersWaiting_;

  /**
     resets the cycle timer
   */
  void resetCycleTimer();

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
  */
  void moveFuel(MatBuff& fromBuff, MatBuff& toBuff, double amt);

  /**
     move all fuel from one buffer to another
  */
  void moveFuel(MatBuff& fromBuff, MatBuff& toBuff);

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
class CycBatchReactorDestructException : public CycException {
 public: 
 CycBatchReactorDestructException(std::string msg) : CycException(msg) {};
};

/**
   An exception class for BatchReactors that exhibit undefined behavior
   for a given phase
*/
class CycBatchReactorPhaseBehaviorException : public CycException {
 public: 
 CycBatchReactorPhaseBehaviorException(std::string msg) : CycException(msg) {};
};

#endif
