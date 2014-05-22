// batch_reactor.h
#ifndef CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_
#define CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_

#include <map>
#include <queue>
#include <string>

#include "cyclus.h"

// forward declarations
namespace cycamore {
class BatchReactor;
}  // namespace cycamore
namespace cyclus {
class Context;
}  // namespace cyclus

namespace cycamore {

/// @class BatchReactor
///
/// @section introduction Introduction
/// The BatchReactor is a facility that agents batch processing. It has three
/// storage areas which hold batches of materials: reserves, core, and
/// storage. Incoming material orders are placed into reserves, from which the
/// core is provided batches during refueling. When a process has been
/// completed, batches are moved from the core into storage. Requests for
/// material are bid upon based on the state of the material in storage.
///
/// The Reactor can manage multiple input-output commodity pairs, and keeps
/// track of the pair that each batch belongs to. Batches move through the
/// system independently of their input/output commodity types, but when batches
/// reach the storage area, they are offered as bids dependent on their output
/// commodity type.
///
/// @section params Parameters
/// A BatchReactor has the following tunable parameters:
///   #. batch_size : the size of batches
///   #. n_batches : the number of batches that constitute a full core
///   #. process_time : the number of timesteps a batch process takes
///   #. n_load : the number of batches processed at any given time (i.e.,
///   n_load is unloaded and reloaded after a process is finished
///   #. n_reserves : the preferred number of batches in reserve
///   #. preorder_time : the amount of time before a process is finished to
///   order fuel
///   #. refuel_time : the number of timesteps required to reload the core after
///   a process has finished
///
/// The BatchReactor also maintains a cyclus::CommodityRecipeContext, which
/// allows it to track incommodity-inrecipe/outcommodity-outrecipe groupings.
///
/// @section operation Operation
/// After a BatchReactor enters the simulation, it will begin processing its
/// first batch load on the Tick after its core has been filled.
///
/// It will maintain its "processing" state for process_time() time steps,
/// including the timestep on which it began. It will unload n_load() batches
/// from its core on the Tock of that time step. For example, if a reactor
/// begins its process at time 1 and has a process_time equal to 10, it will
/// unload batches on the Tock of time step 10.
///
/// Starting at the next time step, the reactor will attempt to refuel itself
/// from whatever batches exist in its reserves container (i.e, already-ordered
/// fuel). Assuming its core buffer has been refueled, it will wait reload_time
/// timesteps. On the tick of the following timestep, the process will begin
/// again. Using the previous example, assume that the refuel_time is equal to
/// two and that the core buffer has been refueled appropriately. The refueling
/// "phase" will begin on time step 11, and will end on the Tock of time step
/// 12. The process will begin again on time step 13 (analogous to its state
/// originally at time step 1).
///
/// @section end End of Life
/// If the current time step is equivalent to the facility's lifetime, the
/// reactor will move all material in its core to its storage containers.
///
/// @section requests Requests
/// A BatchReactor will make as many requests as it has possible input
/// commodities. It provides a constraint based on a total request amount
/// determined by its batch_size, n_load, and n_reserves parameters. The
/// n_reserves parameter allows agenters to order fuel in advance of when it is
/// needed. The fuel order size is batch_size * (n_load + n_reserves). These
/// requests are made if the current simulation time is less than or equal to
/// the reactor's current order_time(), which is determined by the ending time
/// of the current process less a look ahead time, the preorder_time().
///
/// A special case exists when the reactor first enters the simulation, where it
/// will order as much fuel as is needed to fill its full core.
///
/// @section bids Bids
/// A BatchReactor will bid on any request for any of its out_commodities, as
/// long as there is a positive quantity of material in its storage area
/// associated with that output commodity.
///
/// @section ics Initial Conditions
/// A BatchReactor can be deployed with any number of batches in its reserve,
/// core, and storage buffers. Recipes and commodities for each of these batch
/// groupings must be specified.
///
/// @todo add decommissioning behavior if material is still in storage
///
/// @warning preference time changing is based on *full simulation time*, not
/// relative time
/// @warning the reactor's commodity context *can not* currently remove
/// resources reliably because of toolkit::ResourceBuff::PopQty()'s implementation.
/// Resource removal from the context requires pointer equality
/// in order to remove material, and PopQty will split resources, making new
/// pointers.
/// @warning the reactor uses a hackish way to input materials into its
/// reserves. See the AddBatches_ member function.
class BatchReactor
    : public cyclus::Facility,
      public cyclus::toolkit::CommodityProducer {
 public:
  /// @brief defines all possible phases this facility can be in
  enum Phase {
    INITIAL,  ///< The initial phase, after the facility is built but before it is
              /// filled
    PROCESS,  ///< The processing phase
    WAITING,  ///< The waiting phase, while the facility is waiting for fuel
              /// between processes
  };

  /// @brief a struct for initial conditions
  struct InitCond {
    InitCond() : n_reserves(0), n_core(0), n_storage(0) {}

    void AddReserves(int n, std::string rec, std::string commod) {
      n_reserves = n;
      reserves_rec = rec;
      reserves_commod = commod;
    }

    void AddCore(int n, std::string rec, std::string commod) {
      n_core = n;
      core_rec = rec;
      core_commod = commod;
    }

    void AddStorage(int n, std::string rec, std::string commod) {
      n_storage = n;
      storage_rec = rec;
      storage_commod = commod;
    }

    int n_reserves;
    std::string reserves_rec;
    std::string reserves_commod;

    int n_core;
    std::string core_rec;
    std::string core_commod;

    int n_storage;
    std::string storage_rec;
    std::string storage_commod;
  };

  /* --- Module Members --- */
  /// @param ctx the cyclus context for access to simulation-wide parameters
  BatchReactor(cyclus::Context* ctx);

  virtual ~BatchReactor();

  virtual cyclus::Agent* Clone();

  virtual void InfileToDb(cyclus::InfileTree* qe, cyclus::DbInit di);

  virtual void InitFrom(cyclus::QueryableBackend* b);

  virtual void Snapshot(cyclus::DbInit di);

  virtual void InitInv(cyclus::Inventories& invs);

  virtual cyclus::Inventories SnapshotInv();

  virtual std::string schema();

  /// initialize members from a different agent
  void InitFrom(BatchReactor* m);

  /// Print information about this agent
  virtual std::string str();
  /* --- */

  /* --- Facility Members --- */
  /// perform module-specific tasks when entering the simulation 
  virtual void Build(cyclus::Agent* parent);
  /* --- */

  /* --- Agent Members --- */
  /// The Tick function specific to the BatchReactor.
  /// @param time the time of the tick
  virtual void Tick(int time);

  /// The Tick function specific to the BatchReactor.
  /// @param time the time of the tock
  virtual void Tock(int time);
  
  /// notify commodity producer parent of entrance
  virtual void EnterNotify();

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

  /* --- BatchReactor Members --- */
  /// @return the total number of batches in storage
  int StorageCount();

  /// @brief the processing time required for a full batch process before
  /// refueling
  inline void process_time(int t) {
    process_time_ = t;
  }
  inline int process_time() const {
    return process_time_;
  }

  /// @brief the time it takes to refuel
  inline void refuel_time(int t) {
    refuel_time_ = t;
  }
  inline int refuel_time() const {
    return refuel_time_;
  }

  /// @brief the amount of time an order should be placed for new fuel before a
  /// process is finished
  inline void preorder_time(int t) {
    preorder_time_ = t;
  }
  inline int preorder_time() const {
    return preorder_time_;
  }

  /// @brief the starting time of the last (current) process
  inline void start_time(int t) {
    start_time_ = t;
  }
  inline int start_time() const {
    return start_time_;
  }

  /// @brief the ending time of the last (current) process
  /// @warning the - 1 is to ensure that a 1 period process time that begins on
  /// the tick ends on the tock
  inline int end_time() const {
    return start_time() + process_time() - 1;
  }

  /// @brief the beginning time for the next phase, set internally
  inline int to_begin_time() const {
    return to_begin_time_;
  }

  /// @brief the time orders should be taking place for the next refueling
  inline int order_time() const {
    return end_time() - preorder_time();
  }

  /// @brief the number of batches in a full reactor
  inline void n_batches(int n) {
    n_batches_ = n;
  }
  inline int n_batches() const {
    return n_batches_;
  }

  /// @brief the number of batches in reactor refuel loading/unloading
  inline void n_load(int n) {
    n_load_ = n;
  }
  inline int n_load() const {
    return n_load_;
  }

  /// @brief the preferred number of fresh fuel batches to keep in reserve
  inline void n_reserves(int n) {
    n_reserves_ = n;
  }
  inline int n_reserves() const {
    return n_reserves_;
  }

  /// @brief the number of batches currently in the reactor
  inline int n_core() const {
    return core_.count();
  }

  /// @brief the size of a batch
  inline void batch_size(double size) {
    batch_size_ = size;
  }
  inline double batch_size() {
    return batch_size_;
  }

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }
  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /// @brief this facility's initial conditions
  inline void ics(const InitCond& ics) {
    ics_ = ics;
  }
  inline InitCond ics() const {
    return ics_;
  }

  /// @brief the current phase
  void phase(Phase p);
  inline Phase phase() const {
    return phase_;
  }

  /// @brief this facility's preference for input commodities
  inline void commod_prefs(const std::map<std::string, double>& prefs) {
    commod_prefs_ = prefs;
  }
  inline const std::map<std::string, double>& commod_prefs() const {
    return commod_prefs_;
  }

 protected:
  /// @brief moves a batch from core_ to storage_
  virtual void MoveBatchOut_();

  /// @brief gets bids for a commodity from a buffer
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids_(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests,
      std::string commod,
      cyclus::toolkit::ResourceBuff* buffer);

  /// @brief returns a qty of material from a buffer
  cyclus::Material::Ptr TradeResponse_(double qty,
                                       cyclus::toolkit::ResourceBuff* buffer);

  /// @brief a cyclus::toolkit::ResourceBuff for material while they are inside the core,
  /// with all materials guaranteed to be of batch_size_
  cyclus::toolkit::ResourceBuff core_;

  /// @brief a cyclus::toolkit::ResourceBuff for material once they leave the core.
  /// there is one storage for each outcommodity
  /// @warning no guarantee can be made to the size of each item in storage_, as
  /// requests can be met that are larger or smaller than batch_size_
  std::map<std::string, cyclus::toolkit::ResourceBuff> storage_;

 private:
  /// @brief refuels the reactor until it is full or reserves_ is out of
  /// batches. If the core is full after refueling, the Phase is set to PROCESS.
  void Refuel_();

  /// @brief moves a batch from reserves_ to core_
  void MoveBatchIn_();

  /// @brief construct a request portfolio for an order of a given size
  cyclus::RequestPortfolio<cyclus::Material>::Ptr GetOrder_(double size);

  /// @brief Add a blob of incoming material to reserves_
  ///
  /// The last material to join reserves_ is first investigated to see if it is
  /// of batch_size_. If not, material from mat is added to it and it is
  /// returned to reserves_. If more material remains, chunks of batch_size_ are
  /// removed and added to reserves_. The final chunk may be <= batch_size_.
  void AddBatches_(std::string commod, cyclus::Material::Ptr mat);

  /// @brief adds phase names to phase_names_ map
  void SetUpPhaseNames_();

  static std::map<Phase, std::string> phase_names_;
  int process_time_;
  int preorder_time_;
  int refuel_time_;
  int start_time_;
  int to_begin_time_;
  int n_batches_;
  int n_load_;
  int n_reserves_;
  double batch_size_;
  Phase phase_;

  InitCond ics_;

  cyclus::toolkit::CommodityRecipeContext crctx_;

  /// @warning as is, the int key is **simulation time**, i.e., context()->time
  /// == key. This should be fixed for future use!
  std::map<int, std::vector< std::pair< std::string, std::string > > >
      recipe_changes_;

  /// @brief preferences for each input commodity
  std::map<std::string, double> commod_prefs_;

  /// @warning as is, the int key is **simulation time**, i.e., context()->time
  /// == key. This should be fixed for future use!
  std::map<int, std::vector< std::pair< std::string, double > > > pref_changes_;

  /// @brief allows only batches to enter reserves_
  cyclus::Material::Ptr spillover_;

  /// @brief a cyclus::toolkit::ResourceBuff for material before they enter the core,
  /// with all materials guaranteed to be of batch_size_
  cyclus::toolkit::ResourceBuff reserves_;

  friend class BatchReactorTest;
  /* --- */
};

}  // namespace cycamore

#endif  // CYCAMORE_MODELS_BATCHREACTOR_BATCH_REACTOR_H_
