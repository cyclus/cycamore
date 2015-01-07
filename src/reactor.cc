#include "reactor.h"

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
  if (cycle_step >= cycle_time) {
    Load();
  }

  if (cycle_step < cycle_time + refuel_time || core.space() < cyclus::eps()) {
    // move forward if in cycle operation or refueling, but halt if waiting
    // for fuel at beginning of cycle.
    cycle_step++;
  }
  if (cycle_step > cycle_time + refuel_time) {
    cycle_step = 0;
  }
}

void Reactor::Transmute() {
  MatVect old;
  if (discrete_mode()) {
    old = core.PopN(assem_per_discharge());
  } else {
    old = core.PopQty(assem_per_discharge() * assem_size);
  }
  MatVect tail = core.PopN(core.count());

  for (int i = 0; i < old.size(); i++) {
    int id = old[i]->obj_id();
    old[i]->Transmute(context()->GetRecipe(fuel_outrecipe(id)));
  }
  core.Push(old);
  core.Push(tail);
}

void Reactor::Discharge() {
  double qty_pop = assem_per_discharge() * assem_size;
  if (spent.space() < qty_pop) {
    return; // not enough space in spent fuel inventory
  }

  MatVect old;
  if (discrete_mode()) {
    old = core.PopN(assem_per_discharge());
  } else {
    old = core.PopQty(qty_pop);
  }

  spent.Push(old);
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

} // namespace cycamore

double Reactor::assem_per_discharge() {
  if (discrete_mode()) {
    return static_cast<int>(n_assem_core / n_batches);
  } else {
    return static_cast<double>(n_assem_core) / n_batches;
  }
}

