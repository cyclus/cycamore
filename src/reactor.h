#ifndef CYCAMORE_SRC_REACTOR_H_
#define CYCAMORE_SRC_REACTOR_H_

#include "cyclus.h"

using cyclus::Material;
using cyclus::toolkit::ResBuf;

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
// * integral_assem (bool, default true) - true if requests require DRE to
//   match all or nothing of each assembly.  Note that this is different from
//   and roughly orthogonal to whether the reactor is operating in discrete or
//   continuous assembly mode.  “True” is the recommended value for this
//   variable.  “False” can result in materials being combined resulting in
//   incorrect transmutations.
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
// fuel, it discretizes the material into assembly quanta (of size
// assem_size).  Discrete assembly mode is the recommended mode of operation
// for a reactor.  If a partial assembly is received, it is stored until
// enough material can be acquired to complete the full assembly.  In this
// mode, partial assemblies may not be inserted into the core.  When in this
// mode, resources will never be split/combined by the reactor post assembly
// discretization.  If integral_assem is false, then this discretization will
// happen upon extraction of an assembly’s worth of material from the fresh
// inventory for placement into the core.  If integral_assem is true, then
// this discretization will happen automatically as part of resource exchange.
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
// extracted/split from the (not assembly discretized) fresh fresh fuel buffer
// (or requested on DRE if no fresh) and inserted into the core.
//
// Time to make it code generatable? - Yes  Bring into compliance? - Yes.
// The BatchReactor currently has no state variables. A UINT type might be
// useful here.  We should scavenge the batch reactor for pieces as we
// reimplement a new code-generated version.
//
// * integral_assem == true, discrete assembly mode - discretization is automatic - Pop() from fresh buffer.
//
// * integral_assem == false, discrete assembly mode - discretization upon extraction from fresh buffer.  Just ExtractQty from fresh buffer
//
// * integral_assem == true, continuous assembly mode - discretization doesn’t matter. Just ExtractQty from fresh buffer.
//
// * integral_assem == false, continuous assembly mode - discretization doesn’t matter.  Just ExtractQty from fresh buffer.
//
// Meta-data associated with resource objects in inventory (e.g. the in-commod
// on which they were received) should be associated with resources based on
// obj_id.  This is necessary for handling the case where integral_assem is
// true in discrete assembly mode where resources are combined inside buffers
// during extraction.

class Reactor : public cyclus::Facility {
 public:
  Reactor(cyclus::Context* ctx);
  virtual ~Reactor() {};

  virtual void EnterNotify() {};
  virtual void Tick() {};
  virtual void Tock() {};

  #pragma cyclus

 private:
  std::string fuel_outcommod(int obj_id);
  std::string fuel_inrecipe(int obj_id);
  std::string fuel_outrecipe(int obj_id);
  std::string fuel_pref(int obj_id);
  /// Returns true if the reactor is operating in discrete assembly mode.
  bool discrete_mode() {return n_assem_core > 1 || n_batches == 1};

  /// discharge a batch from the core
  void Discharge();

  /// top up core inventory if possible
  void Load();

  double assem_per_discharge();

  // inventory and core params
  #pragma cyclus var {}
  double n_batches;
  #pragma cyclus var {}
  double assem_size;
  #pragma cyclus var {"default": 1}
  bool integral_assem;
  #pragma cyclus var {"default": 'n_batches'}
  int n_assem_core;
  #pragma cyclus var {"default": 1000000000}
  int n_assem_spent;
  #pragma cyclus var {"default": 'n_assem_core'}
  int n_assem_fresh;

  // cycle params
  #pragma cyclus var {}
  int cycle_time;
  #pragma cyclus var {}
  int refuel_time;
  #pragma cyclus var { \
    "default": 0, \
    "doc": "number of time steps since the beginning of the last cycle", \
  }
  int cycle_step;

  // fuel specifications
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> fuel_incommods;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
  }
  std::vector<std::string> fuel_inrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "recipe"], \
  }
  std::vector<std::string> fuel_outrecipes;
  #pragma cyclus var { \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> fuel_outcommods;
  #pragma cyclus var {"default": []} // if this is empty, assume zero for all
  std::vector<double> fuel_prefs;

  // This variable should be hidden/unavailable in ui.  Maps resource object
  // id's to the incommods through which they were received.
  #pragma cyclus var {}
  std::map<int, std::string> res_commods;

  // Resource inventories
  #pragma cyclus var {'capacity': 'n_assem_fresh * assem_size'}
  ResBuf<Material> fresh;
  #pragma cyclus var {'capacity': 'n_assem_core * assem_size'}
  ResBuf<Material> core;
  #pragma cyclus var {'capacity': 'n_assem_spent * assem_size'}
  ResBuf<Material> spent;

  // preference changes
  #pragma cyclus var {"default": []}
  std::vector<int> pref_change_times;
  #pragma cyclus var {"default": []}
  std::vector<std::string> pref_change_commods;
  #pragma cyclus var {"default": []}
  std::vector<double> pref_change_values;

  // recipe changes
  #pragma cyclus var {"default": []}
  std::vector<int> recipe_change_times;
  #pragma cyclus var {"default": []}
  std::vector<std::string> recipe_change_commods;
  #pragma cyclus var {"default": []}
  std::vector<std::string> recipe_change_incommods;
  #pragma cyclus var {"default": []}
  std::vector<std::string> recipe_change_outcommods;

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_REACTOR_H_
