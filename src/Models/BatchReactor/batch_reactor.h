// batch_reactor.h
#ifndef CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_
#define CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_

#include <string>
#include <queue>
#include <map>

#include "bid_portfolio.h"
#include "capacity_constraint.h"
#include "commodity_producer.h"
#include "exchange_context.h"
#include "enrichment.h"
#include "facility_model.h"
#include "resource_buff.h"
#include "request_portfolio.h"
#include "transaction.h"
#include "material.h"

// forward declarations
namespace cycamore {
class BatchReactor;
} // namespace cycamore
namespace cyclus {  
class Context;
} // namespace cyclus

namespace cycamore {

/// Defines all possible phases this facility can be in
enum Phase {
  INITIAL, ///< The initial phase, after the facility is built but before it is
           ///filled
  PROCESS, ///< The processing phase
  WAITING, ///< The waiting phase, while the factility is waiting for fuel
           ///between processes
};

/// @class BatchReactor
///
/// @brief The BatchReactor is a facility that models batch processing.
///
/// @todo finish documentation..
/// @todo add a refueling delay
class BatchReactor : public cyclus::FacilityModel,
  public cyclus::CommodityProducer {
 public:
  /* --- Module Members --- */
  /// @param ctx the cyclus context for access to simulation-wide parameters
  BatchReactor(cyclus::Context* ctx);
  
  virtual ~BatchReactor();
  
  virtual cyclus::Model* Clone();
  
  virtual std::string schema();

  /// Initialize members related to derived module class
  /// @param qe a pointer to a cyclus::QueryEngine object containing initialization data
  virtual void InitModuleMembers(cyclus::QueryEngine* qe);

  /// Print information about this model
  virtual std::string str();
  /* --- */

  /* --- Facility Members --- */
  /// perform module-specific tasks when entering the simulation 
  virtual void Deploy(cyclus::Model* parent);

  /// facilities over write this method if a condition must be met
  /// before their destructors can be called
  virtual bool CheckDecommissionCondition();
  /* --- */

  /* --- Agent Members --- */  
  /// The HandleTick function specific to the BatchReactor.
  /// @param time the time of the tick
  virtual void HandleTick(int time);
  
  /// The HandleTick function specific to the BatchReactor.
  /// @param time the time of the tock
  virtual void HandleTock(int time);
  
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
      GetMatlBids(const cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);
  
  /// @brief respond to each trade with a material enriched to the appropriate
  /// level given this facility's inventory
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void PopulateMatlTradeResponses(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);
  /* --- */

  /* --- BatchReactor Members --- */
  /// @return true if the cyclus::ResourceBuffer has at least one batch inside
  /// @param b the buffer to query
  /// @param size the batch size
  inline bool BatchIn(const cyclus::ResourceBuff b, double size) {
    return b.quantity() >= size;
  }
  
  /// @brief the processing time required for a full batch process before
  /// refueling
  inline void process_time(int t) { process_time_ = t; }
  inline int process_time() const { return process_time_; }
  
  /// @brief the amount of time an order should be placed for new fuel before a
  /// process is finished
  inline void preorder_time(int t) { preorder_time_ = t; }
  inline int preorder_time() const { return preorder_time_; }

  /// @brief the starting time of the last (current) process
  inline void start_time(int t) { start_time_ = t; }
  inline int start_time() const { return start_time_; }

  /// @brief the ending time of the last (current) process
  inline int end_time() const { return start_time() + process_time(); }

  /// @brief the time orders should be taking place for the next refueling
  inline int order_time() const { return end_time() - preorder_time(); }

  /// @brief the number of batches in a full reactor
  inline void n_batches(int n) { n_batches_ = n; }
  inline int n_batches() const { return n_batches_; }

  /// @brief the number of batches in reactor refuel loading/unloading
  inline void n_load(int n) { n_load_ = n; }
  inline int n_load() const { return n_load_; }

  /// @brief the preferred number of fresh fuel batches to keep in reserve
  inline void n_reserves(int n) { n_reserves_ = n; }
  inline int n_reserves() const { return n_reserves_; }

  /// @brief the number of batches currently in the reactor
  inline int n_core() const { return core_.count(); }

  /// @brief the size of an input batch 
  inline void in_batch_size(double size) { in_batch_size_ = size_; }
  inline double in_batch_size() { return in_batch_size_; }

  /// @brief the size of an output batch
  inline void out_batch_size(double size) { out_batch_size_ = size_; }
  inline double out_batch_size() const { return out_batch_size_; }

  /// @brief the input commodity
  inline void in_commodity(std::string name) { in_commodity_ = name; }
  inline std::string in_commodity() const { return in_commodity_; }

  /// @brief the input recipe
  inline void in_recipe(std::string name) { in_recipe_ = name; }
  inline std::string in_recipe() const { return in_recipe_; }
  
  /// @brief the output commodity
  inline void out_commodity(std::string name) { out_commodity_ = name; }
  inline std::string out_commodity() const { return out_commodity_; }
  
  /// @brief the output recipe
  inline void out_recipe(std::string name) { out_recipe_ = name; }
  inline std::string out_recipe() const { return out_recipe_; }
  
  /// @brief the current phase
  void phase(Phase p);
  inline Phase phase() const { return phase_; }

 private:
  /// @brief refuels the reactor until it is full or reserves_ is out of
  /// batches. If the core is full after refueling, the Phase is set to PROCESS.
  void Refuel_();
  
  static std::map<Phase, std::string> phase_names_;
  int process_time_;
  int preorder_time_;
  int start_time_;
  int n_batches_;
  int n_load_;
  int n_reserves_;
  double in_batch_size_;
  double out_batch_size_;
  std::string in_commodity_;
  std::string out_commodity_;
  std::string in_recipe_;
  std::string out_recipe_;
  Phase phase_;
  
  /// a matbuff for material before they enter the core
  cyclus::ResourceBuff reserves_;
  
  /// a matbuff for material while they are inside the core
  cyclus::ResourceBuff core_;

  /// a matbuff for material after they exit the core
  cyclus::ResourceBuff storage_;
  
  ///    /// populate the phase name map
   
  /// void SetUpPhaseNames();

  
  ///    /// resets the cycle timer
         
  /// void recycle_timer();

  
  ///    /// return true if the cycle timer is >= the
  ///    /// cycle length
   
  /// bool CycleComplete();

  
  ///    /// return true if the core is filled
   
  /// bool CoreFilled();

  
  ///    /// set the next phase
  ///    /// @param p the next phase
   
  /// void SetPhase(Phase p);

  
  ///    /// make reqest for a specific amount of fuel
   
  /// void MakeRequest(double amt);

  
  ///    /// offer all off-loaded fuel
   
  /// void MakeOffers();

  
  ///    /// sends a request of offer to the commodity's market
   
  /// void interactWithMarket(std::string commod, double amt, cyclus::TransType type);

  
  ///    /// Processes all orders in ordersWaiting_
   
  /// void HandleOrders();

  
  ///    /// load fuel from preCore_ into inCore_
   
  /// void LoadCore();

  
  ///    /// move a batch from inCore_ to postCore_
   
  /// void OffloadBatch();

  
  ///    /// move all material from inCore_ to postCore_
   
  /// void OffloadCore();
  /* --- */
};

} // namespace cycamore

#endif // CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_
