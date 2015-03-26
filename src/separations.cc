#include "separations.h"

using cyclus::Material;
using cyclus::Composition;
using cyclus::toolkit::ResBuf;
using cyclus::toolkit::MatVec;
using cyclus::KeyError;
using cyclus::ValueError;
using cyclus::Request;
using cyclus::CompMap;

namespace cycamore {

Separations::Separations(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Separations archetype "
                                             "is experimental");
}

cyclus::Inventories Separations::SnapshotInv() {
  cyclus::Inventories invs;

  invs["leftover"] = leftover.PopNRes(leftover.count());
  leftover.Push(invs["leftover"]);
  invs["feed"] = feed.PopNRes(feed.count());
  feed.Push(invs["feed"]);

  std::map<std::string, ResBuf<Material> >::iterator it;
  for (it = streambufs.begin(); it != streambufs.end(); ++it) {
    invs[it->first] = it->second.PopNRes(it->second.count());
    it->second.Push(invs[it->first]);
  }

  return invs;
}

void Separations::InitInv(cyclus::Inventories& inv) {
  leftover.Push(inv["leftover"]);
  feed.Push(inv["feed"]);

  cyclus::Inventories::iterator it;
  for (it = inv.begin(); it != inv.end(); ++it) {
    streambufs[it->first].Push(it->second);
  }
}

typedef std::pair<double, std::map<int, double> > Stream;
typedef std::map<std::string, Stream> StreamSet;

void Separations::EnterNotify() {
  cyclus::Facility::EnterNotify();
  StreamSet::iterator it;
  for (it = streams_.begin(); it != streams_.end(); ++it) {
    std::string name = it->first;
    Stream stream = it->second;
    double cap = stream.first;
    if (cap >= 0) {
      streambufs[name].capacity(cap);
    }
  }
}

void Separations::Tick() {
  Material::Ptr mat = feed.Pop(std::min(throughput, feed.quantity()));

  StreamSet::iterator it;
  double maxfrac = 1;
  std::map<std::string, Material::Ptr> stagedsep;
  for (it = streams_.begin(); it != streams_.end(); ++it) {
    Stream info = it->second;
    std::string name = it->first;
    stagedsep[name] = SepMaterial(info.second, mat);
    double frac = streambufs[name].space() / stagedsep[name]->quantity();
    if (frac < maxfrac) {
      maxfrac = frac;
    }
  }

  std::map<std::string, Material::Ptr>::iterator itf;
  for (itf = stagedsep.begin(); itf != stagedsep.end(); ++it) {
    std::string name = itf->first;
    Material::Ptr m = itf->second;
    streambufs[name].Push(mat->ExtractComp(m->quantity() * maxfrac, m->comp()));
  }

  // push back any leftover onto feed stocks
  if (maxfrac < 1) {
    feed.Push(mat);
  }
}

// Note that this returns an untracked material that should just be used for
// its composition and qty - not in any real inventories, etc.
Material::Ptr SepMaterial(std::map<int, double> effs, Material::Ptr mat) {
  CompMap cm = mat->comp()->mass();
  cyclus::compmath::Normalize(&cm, mat->quantity());
  double tot_qty = 0;
  CompMap sepcomp;

  CompMap::iterator it;
  for (it = cm.begin(); it != cm.end(); ++it) {
    int nuc = it->first;
    int elem = (nuc / 10000000) * 10000000;
    double eff = 0;
    if (effs.count(nuc) > 0) {
      eff = effs[nuc];
    } else if (effs.count(elem) > 0) {
      eff = effs[elem];
    } else {
      continue;
    }
    
    double qty = it->second;
    double sepqty = qty * eff;
    sepcomp[nuc] = sepqty;
    tot_qty += sepqty;
  }

  Composition::Ptr c = Composition::CreateFromMass(sepcomp);
  return Material::CreateUntracked(tot_qty, c);
};

std::set<cyclus::RequestPortfolio<Material>::Ptr>
Separations::GetMatlRequests() {
  using cyclus::RequestPortfolio;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  //Material::Ptr m;

  //int n_assem_order = n_assem_core - core.count()
  //                    + n_assem_fresh - fresh.count();
  //if (n_assem_order == 0) {
  //  return ports;
  //}

  //for (int i = 0; i < n_assem_order; i++) {
  //  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  //  std::vector<Request<Material>*> mreqs;
  //  for (int j = 0; j < fuel_incommods.size(); j++) {
  //    std::string commod = fuel_incommods[j];
  //    double pref = fuel_prefs[j];
  //    Composition::Ptr recipe = context()->GetRecipe(fuel_inrecipes[j]);
  //    m = Material::CreateUntracked(assem_size, recipe);
  //    Request<Material>* r = port->AddRequest(m, this, commod, pref, true);
  //    mreqs.push_back(r);
  //  }
  //  port->AddMutualReqs(mreqs);
  //  ports.insert(port);
  //}

  return ports;
}

void Separations::GetMatlTrades(
    const std::vector< cyclus::Trade<Material> >& trades,
    std::vector<std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {
  //using cyclus::Trade;

  //std::map<std::string, MatVec> mats = PopSpent();
  //std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  //for (int i = 0; i < trades.size(); i++) {
  //  std::string commod = trades[i].request->commodity();
  //  Material::Ptr m = mats[commod].back();
  //  mats[commod].pop_back();
  //  responses.push_back(std::make_pair(trades[i], m));
  //  res_indexes.erase(m->obj_id());
  //}
  //PushSpent(mats); // return leftovers back to spent buffer
}

void Separations::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {

  //std::vector< std::pair<cyclus::Trade<cyclus::Material>,
  //                       cyclus::Material::Ptr> >::const_iterator trade;

  //std::stringstream ss;
  //int nload = std::min((int)responses.size(), n_assem_core - core.count());
  //if (nload > 0) {
  //  ss << nload << " assemblies";
  //  Record("LOAD", ss.str());
  //}

  //for (trade = responses.begin(); trade != responses.end(); ++trade) {
  //  std::string commod = trade->first.request->commodity();
  //  Material::Ptr m = trade->second;
  //  index_res(m, commod);

  //  if (core.count() < n_assem_core) {
  //    core.Push(m);
  //  } else {
  //    fresh.Push(m);
  //  }
  //}
}

std::set<cyclus::BidPortfolio<Material>::Ptr>
Separations::GetMatlBids(cyclus::CommodMap<Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;

  std::set<BidPortfolio<Material>::Ptr> ports;

  //bool gotmats = false;
  //std::map<std::string, MatVec> all_mats;
  //for (int i = 0; i < fuel_outcommods.size(); i++) {
  //  std::string commod = fuel_outcommods[i];
  //  std::vector<Request<Material>*>& reqs = commod_requests[commod];
  //  if (reqs.size() == 0) {
  //    continue;
  //  } else if (!gotmats) {
  //    all_mats = PeekSpent();
  //  }

  //  MatVec mats = all_mats[commod];
  //  if (mats.size() == 0) {
  //    continue;
  //  }

  //  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  //  for (int j = 0; j < reqs.size(); j++) {
  //    Request<Material>* req = reqs[j];
  //    double tot_bid = 0;
  //    for (int k = 0; k < mats.size(); k++) {
  //      Material::Ptr m = mats[k];
  //      tot_bid += m->quantity();
  //      port->AddBid(req, m, this, true);
  //      if (tot_bid >= req->target()->quantity()) {
  //        break;
  //      }
  //    }
  //  }

  //  double tot_qty = 0;
  //  for (int j = 0; j < mats.size(); j++) {
  //    tot_qty += mats[j]->quantity();
  //  }
  //  cyclus::CapacityConstraint<Material> cc(tot_qty);
  //  port->AddConstraint(cc);
  //  ports.insert(port);
  //}

  return ports;
}

void Separations::Tock() {
}

extern "C" cyclus::Agent* ConstructSeparations(cyclus::Context* ctx) {
  return new Separations(ctx);
}

} // namespace cycamore

