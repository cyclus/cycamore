#include "reactor.h"

using cyclus::Material;
using cyclus::toolkit::ResBuf;
using cyclus::toolkit::MatVec;
using cyclus::KeyError;

namespace cycamore {

Reactor::Reactor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      n_batches(0),
      assem_size(0),
      integral_assem(0),
      n_assem_core(0),
      n_assem_spent(0),
      n_assem_fresh(0),
      cycle_time(0),
      refuel_time(0),
      cycle_step(0) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Reactor archetype "
                                             "is experimental");
}

void Reactor::Tick() {
  if (cycle_step == cycle_time) {
    Transmute();
  } else if (cycle_step == cycle_time + refuel_time) {
    Discharge();
    Load();
  }

  // update preferences

  // update recipes
}

// DRE code here - request enough fuel to fill fresh and core inventories.  Fill core first.

void Reactor::Tock() {
  cycle_step++;

  if (cycle_step >= cycle_time) {
    Load();
  }
  if (cycle_step > cycle_time + refuel_time && core.space() < cyclus::eps()) {
    cycle_step = 0;
  }
}

void Reactor::Transmute() {
  MatVec old;
  if (discrete_mode()) {
    old = core.PopN(assem_per_discharge());
  } else {
    old.push_back(core.Pop(assem_per_discharge() * assem_size));
  }

  for (int i = 0; i < old.size(); i++) {
    old[i]->Transmute(context()->GetRecipe(fuel_outrecipe(old[i])));
  }

  MatVec tail = core.PopN(core.count());
  core.Push(old);
  core.Push(tail);
}

bool Reactor::Discharge() {
  double qty_pop = assem_per_discharge() * assem_size;
  if (spent.space() < qty_pop) {
    return false; // not enough space in spent fuel inventory
  }

  if (discrete_mode()) {
    spent.Push(core.PopN(assem_per_discharge()));
  } else {
    // Transmute already discretized the batch to extract into a single object
    spent.Push(core.Pop());
  }
  return true;
}

void Reactor::Load() {
  if (core.space() < cyclus::eps()) {
    return; // core is full
  }

  if (discrete_mode()) {
    while (core.space() > assem_size && fresh.quantity() >= assem_size) {
      if (integral_assem) {
        core.Push(fresh.Pop());
      } else {
        core.Push(fresh.Pop(assem_size));
      }
    }
  } else {
    core.Push(fresh.Pop(std::min(core.space(), fresh.quantity())));
  }
}

double Reactor::assem_per_discharge() {
  if (discrete_mode()) {
    return static_cast<int>(n_assem_core / n_batches);
  } else {
    return static_cast<double>(n_assem_core) / n_batches;
  }
}

std::string Reactor::fuel_incommod(Material::Ptr m) {
  int i = res_indexes[m->obj_id()];
  if (i >= fuel_incommods.size()) {
    throw KeyError("cycamore::Reactor - no incommod for material object");
  }
  return fuel_incommods[i];
}

std::string Reactor::fuel_outcommod(Material::Ptr m) {
  int i = res_indexes[m->obj_id()];
  if (i >= fuel_outcommods.size()) {
    throw KeyError("cycamore::Reactor - no outcommod for material object");
  }
  return fuel_outcommods[i];
}

std::string Reactor::fuel_inrecipe(Material::Ptr m) {
  int i = res_indexes[m->obj_id()];
  if (i >= fuel_inrecipes.size()) {
    throw KeyError("cycamore::Reactor - no inrecipe for material object");
  }
  return fuel_inrecipes[i];
}

std::string Reactor::fuel_outrecipe(Material::Ptr m) {
  int i = res_indexes[m->obj_id()];
  if (i >= fuel_outrecipes.size()) {
    throw KeyError("cycamore::Reactor - no outrecipe for material object");
  }
  return fuel_outrecipes[i];
}

double Reactor::fuel_pref(Material::Ptr m) {
  int i = res_indexes[m->obj_id()];
  if (i >= fuel_prefs.size()) {
    return 0;
  }
  return fuel_prefs[i];
}

} // namespace cycamore

