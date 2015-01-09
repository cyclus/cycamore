#include "reactor.h"

using cyclus::Material;
using cyclus::Composition;
using cyclus::toolkit::ResBuf;
using cyclus::toolkit::MatVec;
using cyclus::KeyError;
using cyclus::ValueError;
using cyclus::Request;

namespace cycamore {

Reactor::Reactor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      n_assem_batch(0),
      assem_size(0),
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
  }
  if (cycle_step == cycle_time + refuel_time) {
    Discharge();
    Load();
  }
  
  int t = context()->time();

  // update preferences
  for (int i = 0; i < pref_change_times.size(); i++) {
    int change_t = pref_change_times[i];
    if (t != change_t) {
      continue;
    }

    std::string incommod = pref_change_commods[i];
    for (int j = 0; j < fuel_incommods.size(); j++) {
      if (fuel_incommods[j] == incommod) {
        fuel_prefs[j] = pref_change_values[i];
        break;
      }
    }
  }

  // update recipes
  for (int i = 0; i < recipe_change_times.size(); i++) {
    int change_t = recipe_change_times[i];
    if (t != change_t) {
      continue;
    }

    std::string incommod = recipe_change_commods[i];
    for (int j = 0; j < fuel_incommods.size(); j++) {
      if (fuel_incommods[j] == incommod) {
        fuel_inrecipes[j] = recipe_change_in[i];
        fuel_outrecipes[j] = recipe_change_out[i];
        break;
      }
    }
  }
}

std::set<cyclus::RequestPortfolio<Material>::Ptr>
Reactor::GetMatlRequests() {
  using cyclus::RequestPortfolio;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr m;

  int n_assem_order = n_assem_core - core.count()
                      + n_assem_fresh - fresh.count();
  if (n_assem_order == 0) {
    return ports;
  }

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

    if (core.count() < n_assem_core) {
      core.Push(m);
    } else {
      fresh.Push(m);
    }
  }
}

std::set<cyclus::BidPortfolio<Material>::Ptr>
Reactor::GetMatlBids(cyclus::CommodMap<Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;

  std::set<BidPortfolio<Material>::Ptr> ports;

  for (int i = 0; i < fuel_outcommods.size(); i++) {
    std::string commod = fuel_outcommods[i];
    MatVec mats = SpentResFor(commod);
    std::vector<Request<Material>*>& reqs = commod_requests[commod];
    if (mats.size() == 0 || reqs.size() == 0) {
      continue;
    }

    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    for (int j = 0; j < reqs.size(); j++) {
      Request<Material>* req = reqs[j];
      double tot_bid = 0;
      for (int k = 0; k < mats.size(); k++) {
        Material::Ptr m = mats[k];
        tot_bid += m->quantity();
        port->AddBid(req, m, this, true);
        if (tot_bid >= req->target()->quantity()) {
          break;
        }
      }
    }

    double tot_qty = 0;
    for (int j = 0; j < mats.size(); j++) {
      tot_qty += mats[j]->quantity();
    }
    cyclus::CapacityConstraint<Material> cc(tot_qty);
    port->AddConstraint(cc);
    ports.insert(port);
  }

  return ports;
}

void Reactor::GetMatlTrades(
    const std::vector< cyclus::Trade<Material> >& trades,
    std::vector<std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {
  using cyclus::Trade;

  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (int i = 0; i < trades.size(); i++) {
    std::string commod = trades[i].request->commodity();
    Material::Ptr m = PopSpentRes(commod);
    responses.push_back(std::make_pair(trades[i], m));
    res_indexes.erase(m->obj_id());
  }
}

void Reactor::Tock() {
  // "if" prevents starting cycle after initial deployment until core is full
  // even though cycle_step is its initial zero.
  if (cycle_step > 0 || core.count() == n_assem_core) {
    cycle_step++;
  }

  // The following "if" is necessary here if cycle_time+refuel_time = 1 in
  // case we didn't have a full core in the Tick, but we got enough during
  // resource exchange.  In this case, we want to still burn a batch this
  // timestep - and every time step as long as fuel can be received.
  if (cycle_step == 1 && refuel_time == 0) {
    Transmute();
    Discharge();
  }

  if (cycle_step >= cycle_time) {
    Load();
  }

  if (cycle_step >= cycle_time + refuel_time && core.count() == n_assem_core) {
    cycle_step = 0;
  }
}

void Reactor::Transmute() {
  if (core.count() < n_assem_core) {
    return;
  }

  MatVec old = core.PopN(n_assem_batch);
  MatVec tail = core.PopN(core.count());
  core.Push(old);
  core.Push(tail);

  for (int i = 0; i < old.size(); i++) {
    old[i]->Transmute(context()->GetRecipe(fuel_outrecipe(old[i])));
  }
}

bool Reactor::Discharge() {
  // we do need min's here in case we ever decide to discharge non-fully
  // batches from a core (e.g. discharge entire core at decommissioning).
  double qty_pop = std::min(n_assem_batch * assem_size, core.quantity());
  if (spent.space() < qty_pop) {
    return false; // not enough space in spent fuel inventory
  }

  int npop = std::min(n_assem_batch, core.count());
  spent.Push(core.PopN(npop));
  return true;
}

void Reactor::Load() {
  int n = std::min(n_assem_core - core.count(), fresh.count());
  core.Push(fresh.PopN(n));
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

Material::Ptr Reactor::PopSpentRes(std::string outcommod) {
  MatVec mats = spent.PopN(spent.count());
  Material::Ptr m;
  bool found = false;
  for (int i = 0; i < mats.size(); i++) {
    std::string commod = fuel_outcommod(mats[i]);
    if (!found && commod == outcommod) {
      m = mats[i];
      found = true;
    } else {
      spent.Push(mats[i]);
    }
  }
  return m;
}
  
MatVec Reactor::SpentResFor(std::string outcommod) {
  MatVec mats = spent.PopN(spent.count());
  MatVec found;
  spent.Push(mats);
  for (int i = 0; i < mats.size(); i++) {
    std::string commod = fuel_outcommod(mats[i]);
    if (commod == outcommod) {
      found.push_back(mats[i]);
    }
  }
  return found;
}

extern "C" cyclus::Agent* ConstructReactor(cyclus::Context* ctx) {
  return new Reactor(ctx);
}

} // namespace cycamore

