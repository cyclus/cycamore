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
// * fresh: capacity = n_assem_fresh * assem_size + eps
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
// discretization.  Number of assemblies discharged per cycle is computed:
//
//     floor(n_assem_core / n_batches)
//
// If that number is zero or results in a significantly rounded batch size,
// the reactor will throw an exception or print a warning respectively.
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

  /// Discharge a batch from the core if there is room in the spent fuel
  /// inventory.  Returns true if a batch was successfully discharged.
  bool Discharge();

  /// Top up core inventory as much as possible.
  void Load();

  /// Transmute the batch in the core that is about to be discharged to its
  /// fully burnt state as defined by its outrecipe.
  void Transmute();

  cyclus::toolkit::MatVec SpentResFor(std::string outcommod);

  cyclus::Material::Ptr PopSpentRes(std::string outcommod);

  //////////// inventory and core params ////////////
  #pragma cyclus var { \
    "doc": "", \
  }
  int n_assem_batch;
  #pragma cyclus var { \
    "doc": "", \
  }
  double assem_size;
  #pragma cyclus var { \
    "doc": "", \
  }
  int n_assem_core;
  #pragma cyclus var { \
    "default": 1000000000, \
  }
  int n_assem_spent;
  #pragma cyclus var { \
    "default": 0, \
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
  std::vector<std::string> recipe_change_in;
  #pragma cyclus var { \
    "default": [], \
  }
  std::vector<std::string> recipe_change_out;

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_REACTOR_H_
