#include "reactor.h"

using cyclus::Material;
using cyclus::Composition;
using cyclus::toolkit::ResBuf;
using cyclus::toolkit::MatVec;
using cyclus::KeyError;
using cyclus::ValueError;

namespace cycamore {

Reactor::Reactor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      n_batches(0),
      assem_size(0),
      n_assem_core(0),
      n_batch_spent(0),
      n_batch_fresh(0),
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

std::set<cyclus::RequestPortfolio<Material>::Ptr>
Reactor::GetMatlRequests() {
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr m;

  if (discrete_mode()) {
    int n_assem_order = static_cast<int>(core.space() / assem_size)
                        + static_cast<int>(fresh.space() / assem_size);
    for (int i = 0; i < n_assem_order; i++) {
      std::vector<Request<Material>*> mreqs;
      for (int j = 0; j < fuel_incommods.size(); j++) {
        std::string commod = fuel_incommods[j];
        double pref = fuel_prefs[j];
        Composition::Ptr recipe = context()->GetRecipe(fuel_inrecipes[j]);
        m = Material::CreateUntracked(assem_size, recipe);
        Request<Material>* r = port->AddRequest(m, this, commod, pref, true);
        mreqs.push_back(r);
      }
      port->AddMutualReqs(mreqs);
    }
  } else {
    double qty_order = core.space() + fresh.space();
    std::vector<Request<Material>*> mreqs;
    for (int i = 0; i < fuel_incommods.size(); i++) {
      std::string commod = fuel_incommods[i];
      double pref = fuel_prefs[i];
      Composition::Ptr recipe = context()->GetRecipe(fuel_inrecipes[i]);
      m = Material::CreateUntracked(qty_order, recipe);
      Request<Material>* r = port->AddRequest(m, this, commod, pref);
      mreqs.push_back(r);
    }
    port->AddMutualReqs(mreqs);
  }

  ports.insert(port);
  return ports;
}

void Reactor::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {

  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator trade;

  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    std::string commod = trade->first.request->commodity();
    Material::Ptr m = trade->second;
    index_res(m, commod);

    if (core.space() >= m->quantity()) {
      // discrete or continuous mode with enough room in core for entire mat
      core.Push(m);
    } else if (!discrete_mode()) {
      // continuous mode and must split material between core and fresh bufs
      Material::Ptr m2 = m->ExtractQty(core.space());
      index_res(m2, commod);
      core.Push(m2);
      fresh.Push(m);
    } else {
      // discrete mode and no room in core
      fresh.Push(m);
    }
  }
}

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
  // we don't need min(assem_per_discharge()..., core.quantity()) because
  // this function is+should only be called at the end of an operational cycle -
  // which can only happen if the core is full.
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
  // we do need min's here in case we ever decide to discharge non-fully
  // batches from a core (e.g. discharge entire core at decommissioning).
  double qty_pop = std::min(assem_per_discharge() * assem_size, core.quantity());
  if (spent.space() < qty_pop) {
    return false; // not enough space in spent fuel inventory
  }

  if (discrete_mode()) {
    int npop = std::min((int)assem_per_discharge(), core.count());
    spent.Push(core.PopN(npop));
  } else {
    spent.Push(core.Pop(qty_pop));
  }
  return true;
}

void Reactor::Load() {
  if (core.space() < cyclus::eps()) {
    return; // core is full
  }

  if (discrete_mode()) {
    while (core.space() > assem_size && fresh.quantity() >= assem_size) {
      core.Push(fresh.Pop());
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

void Reactor::index_res(cyclus::Resource::Ptr m, std::string incommod) {
  for (int i = 0; i < fuel_incommods.size(); i++) {
    if (fuel_incommods[i] == incommod) {
      res_indexes[m->obj_id()] = i;
      return;
    }
  }
  throw ValueError("cycamore::Reactor - received unsupported incommod material");
}
  
} // namespace cycamore

