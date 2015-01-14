#ifndef CYCAMORE_SRC_REACTOR_H_
#define CYCAMORE_SRC_REACTOR_H_

#include "cyclus.h"

namespace cycamore {

// Configurable parameters:
//
// * assem_size (double) - the mass in kg of a single assembly
//
// * n_assem_batch (int) - number of assemblies discharged per batch at the
//   end of an operational cycle
//
// * n_assem_core (int) - the number of assemblies that constitute a core.
//
// * n_assem_spent (int, default infinite) - the number of spent fuel assemblies
//   that can be stored
//
// * n_assem_fresh (int, default n_assem_core) - the number of fresh assemblies to keep on
//   hand
//
// The Reactor will have 3 resource buffers (all with automatically computed
// capacities based on above params):
//
// * fresh: capacity = n_assem_fresh * assem_size
//
// * core: capacity = n_assem_core * assem_size
//
// * spent: capacity = n_assem_spent * assem_size
//
// The reactor will always try to keep its fresh_fuel and core buffers full.
// If the fresh fuel buffer has zero capacity (i.e. n_assem_fresh == 0), then
// the reactor will not order new fuel until the next batch is discharged from
// the core - i.e. "just-in-time" ordering.
// The cycle progresses/runs only if the reactor is in active operation. The
// following conditions result in reactor operation being suspended:
//
// * There are not enough full assemblies (with unspent life) to provide a full core
//
// * There is no room in the spent_fuel buffer to discharge a fully-burned assembly into.
//
// A reactor only requests and accepts material in discrete assembly quanta (of size
// assem_size). Resources will never be split/combined by the reactor.
//
// Meta-data associated with resource objects in inventory (e.g. the in-commod
// on which they were received) should be associated with resources based on
// obj_id.  This is necessary for maintaining continuity in the face of
// potential decay calcs and other things that might change the regular
// Resource::id.

class Reactor : public cyclus::Facility {
 public:
  Reactor(cyclus::Context* ctx);
  virtual ~Reactor() {};

  virtual void Tick();
  virtual void Tock();

  void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  virtual void GetMatlTrades(
      const std::vector< cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  #pragma cyclus

 private:
  std::string fuel_incommod(cyclus::Material::Ptr m);
  std::string fuel_outcommod(cyclus::Material::Ptr m);
  std::string fuel_inrecipe(cyclus::Material::Ptr m);
  std::string fuel_outrecipe(cyclus::Material::Ptr m);
  double fuel_pref(cyclus::Material::Ptr m);

  /// Store fuel info index for the given resource received on incommod.
  void index_res(cyclus::Resource::Ptr m, std::string incommod);

  /// Discharge a batch from the core if there is room in the spent fuel
  /// inventory.  Returns true if a batch was successfully discharged.
  bool Discharge();

  /// Top up core inventory as much as possible.
  void Load();

  /// Transmute the batch in the core that is about to be discharged to its
  /// fully burnt state as defined by its outrecipe.
  void Transmute();

  /// Records a reactor event to the output db with the given name and note val.
  void Record(std::string name, std::string val);

  /// Returns pointers to all the spent fuel materials that should be offered
  /// on a particular outcommod without removing them from the spent
  /// fuel buffer.
  cyclus::toolkit::MatVec SpentResFor(std::string outcommod);

  /// Returns a single assembly for the specified outcommod - removing it from
  /// the spent fuel buffer.
  cyclus::Material::Ptr PopSpentRes(std::string outcommod);

  //////////// inventory and core params ////////////
  #pragma cyclus var { \
    "doc": "Number of assemblies that constitute a single batch." \
           "This is the number of assemblies discharged from the core fully burned each cycle." \
           "Batch size is equivalent to ``n_assem_batch / n_assem_core``.", \
  }
  int n_assem_batch;
  #pragma cyclus var { \
    "doc": "Mass (kg) of a single assembly.", \
    "units": "kg", \
  }
  double assem_size;
  #pragma cyclus var { \
    "doc": "Number of assemblies that constitute a full core.", \
  }
  int n_assem_core;
  #pragma cyclus var { \
    "default": 1000000000, \
    "doc": "Number of spent fuel assemblies that can be stored on-site before reactor operation stalls.", \
  }
  int n_assem_spent;
  #pragma cyclus var { \
    "default": 0, \
    "doc": "Number of fresh fuel assemblies to keep on-hand if possible.", \
  }
  int n_assem_fresh;

  ///////// cycle params ///////////
  #pragma cyclus var { \
    "doc": "The duration of a full operational cycle (excluding refueling time) in time steps.", \
    "units": "time steps", \
  }
  int cycle_time;
  #pragma cyclus var { \
    "doc": "The duration of a full refueling period - the minimum time between" \
           " a cycle end and the start of the next cycle.", \
    "units": "time steps", \
  }
  int refuel_time;
  #pragma cyclus var { \
    "default": 0, \
    "doc": "Number of time steps since the start of the last cycle." \
           " Only set this if you know what you are doing", \
    "units": "time steps", \
  }
  int cycle_step;

  /////// fuel specifications /////////
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
    "doc": "Ordered list of input commodities on which to requesting fuel.", \
  }
  std::vector<std::string> fuel_incommods;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
    "doc": "Fresh fuel recipes to request for each of the given fuel input commodities (same order).", \
  }
  std::vector<std::string> fuel_inrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
    "doc": "Spent fuel recipes corresponding to the given fuel input commodities (same order)." \
           " Fuel received via a particular input commodity is transmuted to the recipe specified" \
           " here after being burned during a cycle.", \
  }
  std::vector<std::string> fuel_outrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
    "doc": "Output commodities on which to offer spent fuel originally received as each particular " \ 
           " input commodity (same order)." \
  }
  std::vector<std::string> fuel_outcommods;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The preference for each type of fresh fuel requested corresponding to each input" \
           " commodity (same order).  If no preferences are specified, zero is" \
           " used for all fuel requests (default).", \
  }
  std::vector<double> fuel_prefs;

  // This variable should be hidden/unavailable in ui.  Maps resource object
  // id's to the index for the incommod through which they were received.
  #pragma cyclus var {"default": {}, "doc": "This should NEVER be set manually."}
  std::map<int, int> res_indexes;

  // Resource inventories - these must be defined AFTER/BELOW the member vars
  // referenced (e.g. n_batch_fresh, assem_size, etc.).
  #pragma cyclus var {"capacity": "n_assem_fresh * assem_size"}
  cyclus::toolkit::ResBuf<cyclus::Material> fresh;
  #pragma cyclus var {"capacity": "n_assem_core * assem_size"}
  cyclus::toolkit::ResBuf<cyclus::Material> core;
  #pragma cyclus var {"capacity": "n_assem_spent * assem_size"}
  cyclus::toolkit::ResBuf<cyclus::Material> spent;

  /////////// preference changes ///////////
  #pragma cyclus var { \
    "default": [], \
    "doc": "The time step on which to change the request preference for a " \
           "particular fresh fuel type.", \
  }
  std::vector<int> pref_change_times;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The input commodity for a particular fuel preference change." \
           " Same order as and direct correspondence to the specified preference change time steps.", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> pref_change_commods;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The new/changed request preference for a particular fresh fuel." \
           " Same order as and direct correspondence to the specified preference change time steps.", \
  }
  std::vector<double> pref_change_values;

  ///////////// recipe changes ///////////
  #pragma cyclus var { \
    "default": [], \
    "doc": "The time step on which to change the input-output recipe pair for a requested fresh fuel.", \
  }
  std::vector<int> recipe_change_times;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The input commodity indicating fresh fuel for which recipes will be changed." \
           " Same order as and direct correspondence to the specified recipe change time steps.", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> recipe_change_commods;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The new input recipe to use for this recipe change." \
           " Same order as and direct correspondence to the specified recipe change time steps.", \
    "uitype": ["oneormore", "recipe"], \
  }
  std::vector<std::string> recipe_change_in;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The new output recipe to use for this recipe change." \
           " Same order as and direct correspondence to the specified recipe change time steps.", \
    "uitype": ["oneormore", "recipe"], \
  }
  std::vector<std::string> recipe_change_out;

  // should be hidden in ui (internal only). True if fuel has been discharged
  // this cycle.
  #pragma cyclus var {"default": 0, "doc": "This should NEVER be set manually."}
  bool discharged;
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_REACTOR_H_
