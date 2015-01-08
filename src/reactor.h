#ifndef CYCAMORE_SRC_REACTOR_H_
#define CYCAMORE_SRC_REACTOR_H_

#include "cyclus.h"

namespace cycamore {

// Configurable parameters:
//
// * n_batches (double) - the inverse of the core fraction discharged per cycle
//
// * assem_size (double) - the mass in kg of a single assembly
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
// The cycle progresses/runs only if the reactor is in active operation. The
// following conditions result in reactor operation being suspended:
//
// * There are not enough full assemblies (with unspent life) to provide a full core
//
// * There is no room in the spent_fuel buffer to discharge a fully-burned assembly to.
//
// If a reactor is operating in discrete assembly mode, when it receives fresh
// fuel, it only requests and accepts material in discrete assembly quanta (of size
// assem_size).  Discrete assembly mode is the recommended mode of operation
// for a reactor.  If a partial assembly is received, it is stored until
// enough material can be acquired to complete the full assembly.  In this
// mode, partial assemblies may not be inserted into the core.  When in this
// mode, resources will never be split/combined by the reactor post assembly
// discretization.
// Under either of the following conditions, a reactor will operate in
// discrete assembly mode:
//
// * n_assem_core == n_batches:  pop one assembly per cycle
//
// * n_assem_core > 1:  pop floor(n_assem_core / n_batches) assemblies per cycle
//
// If neither of those conditions are met (i.e. n_assem_core == 1 && n_assem_core !=
// n_batches), then the reactor will operate in continuous assembly mode.  In
// this mode it discharges 1 / n_batches fraction of the single assembly from
// the core at the end of each cycle, and the same quantity will be
// extracted/split from the (not assembly discretized) fresh fuel buffer
// (or requested on DRE if no fresh) and inserted into the core.  Material can
// be requested+accepted in non-assembly sized quanta.
//
// Time to make it code generatable? - Yes  Bring into compliance? - Yes.
// The BatchReactor currently has no state variables. A UINT type might be
// useful here.  We should scavenge the batch reactor for pieces as we
// reimplement a new code-generated version.
//
// Meta-data associated with resource objects in inventory (e.g. the in-commod
// on which they were received) should be associated with resources based on
// obj_id.  This is necessary for handling the case where in continuous
// assembly mode where resources are potentially combined/split inside
// buffers during extraction - and also when decay could change Resource::id.

class Reactor : public cyclus::Facility {
 public:
  Reactor(cyclus::Context* ctx);
  virtual ~Reactor() {};

  virtual void EnterNotify() {};
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

  /// Returns true if the reactor is operating in discrete assembly mode.
  bool discrete_mode() {return n_assem_core > 1 || n_batches == 1;};

  /// Discharge a batch from the core if there is room in the spent fuel
  /// inventory.  Returns true if a batch was successfully discharged.
  bool Discharge();

  /// Top up core inventory as much as possible.
  void Load();

  /// Transmute the batch in the core that is about to be discharged to its
  /// fully burnt state as defined by its outrecipe.
  void Transmute();

  int assem_per_discharge();

  cyclus::toolkit::MatVec SpentResFor(std::string outcommod);

  cyclus::Material::Ptr PopSpentRes(std::string outcommod);

  //////////// inventory and core params ////////////
  #pragma cyclus var { \
    "doc": "", \
  }
  double n_batches;
  #pragma cyclus var { \
    "doc": "", \
  }
  double assem_size;
  #pragma cyclus var { \
    "default": "n_batches", \
  }
  int n_assem_core;
  #pragma cyclus var { \
    "default": 1000000000, \
  }
  int n_assem_spent;
  #pragma cyclus var { \
    "default": 1, \
  }
  int n_assem_fresh;

  ///////// cycle params ///////////
  #pragma cyclus var { \
    "doc": "", \
  }
  int cycle_time;
  #pragma cyclus var { \
    "doc": "", \
  }
  int refuel_time;
  #pragma cyclus var { \
    "default": 0, \
    "doc": "Number of time steps since the beginning of the last cycle.", \
  }
  int cycle_step;

  /////// fuel specifications /////////
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
    "doc": "Ordered list of input commodities for requesting fuel.", \
  }
  std::vector<std::string> fuel_incommods;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
    "doc": "Fresh fuel recipes to request for each of the given fuel_incommods (same order).", \
  }
  std::vector<std::string> fuel_inrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
    "doc": "Spent fuel recipes corresponding to the given fuel_incommods (by order)." \
           " Fuel received via a particular incommod is transmuted to the recipe specified" \
           " here after being burned during a cycle.", \
  }
  std::vector<std::string> fuel_outrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
    "doc": "Output commodities on which to offer spent fuel for each of the given" \ 
           " incommods (same order)." \
  }
  std::vector<std::string> fuel_outcommods;
  #pragma cyclus var { \
    "default": [], \
    "doc": "The preference for each type of fresh fuel requested via the given" \
           " incommods (same order).  If no preferences are specified, zero is" \
           " used for all fuel requests.", \
  }
  std::vector<double> fuel_prefs;

  // This variable should be hidden/unavailable in ui.  Maps resource object
  // id's to the index for the incommod through which they were received.
  #pragma cyclus var {}
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
  }
  std::vector<int> pref_change_times;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<std::string> pref_change_commods;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<double> pref_change_values;

  ///////////// recipe changes ///////////
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<int> recipe_change_times;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<std::string> recipe_change_commods;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<std::string> recipe_change_incommods;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<std::string> recipe_change_outcommods;

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_REACTOR_H_
