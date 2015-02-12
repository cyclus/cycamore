#include "fuel_fab.h"

using cyclus::Material;
using cyclus::Composition;
using pyne::simple_xs;

namespace cycamore {

double CosiWeight(cyclus::Composition::Ptr c, std::string spectrum) {
  cyclus::CompMap cm = c->mass();
  cyclus::compmath::Normalize(&cm);

  double nu_pu239 = 2.85;
  double nu_u233 = 2.5;
  double nu_u235 = 2.4;

  double fiss_u238 = simple_xs("u238", "fission", spectrum);
  double absorb_u238 = simple_xs("u238", "absorption", spectrum);
  double nu_u238 = 0;
  double p_u238 = nu_u238 * fiss_u238 - absorb_u238;

  double fiss_pu239 = simple_xs("Pu239", "fission", spectrum);
  double absorb_pu239 = simple_xs("Pu239", "absorption", spectrum);
  double p_pu239 = nu_pu239 * fiss_pu239 - absorb_pu239;

  cyclus::CompMap::iterator it;
  double w = 0;
  for (it = cm.begin(); it != cm.end(); ++it) {
    cyclus::Nuc nuc = it->first;
    double nu = 0;
    if (nuc == 922350000) {
      nu = nu_u235;
    } else if (nuc == 922330000) {
      nu = nu_u233;
    } else if (nuc == 942390000) {
      nu = nu_pu239;
    }

    double fiss = 0;
    double absorb = 0;
    try {
      fiss = simple_xs(nuc, "fission", spectrum);
      absorb = simple_xs(nuc, "absorption", spectrum);
    } catch(pyne::InvalidSimpleXS err) {
      fiss = 0;
      absorb = 0;
    }

    double p = nu * fiss - absorb;
    w += it->second * (p - p_u238) / (p_pu239 - p_u238);
  }
  return w;
}

double CosiFissileFrac(double w_tgt, double w_fill, double w_fiss) {
  if (w_fiss == w_fill && w_tgt == w_fiss) {
    return 1;
  } else if (w_fiss == w_fill) {
    throw cyclus::ValueError("fissile and filler weights are the same");
  }
  return (w_tgt - w_fill) / (w_fiss - w_fill);
}

double CosiFissileFrac(cyclus::Composition::Ptr target,
                       cyclus::Composition::Ptr filler,
                       cyclus::Composition::Ptr fissile,
                       std::string spectrum) {
  double w_fill = CosiWeight(filler, spectrum);
  double w_fiss = CosiWeight(fissile, spectrum);
  double w_tgt = CosiWeight(target, spectrum);
  return CosiFissileFrac(w_tgt, w_fill, w_fiss);
}

double CosiFillerFrac(double w_tgt, double w_fill, double w_fiss) {
  return 1-CosiFissileFrac(w_tgt, w_fill, w_fiss);
}

// Returns true if the given weights can be used to compute valid mixing
// fractions of the filler and fissile streams to hit the target.
bool CosiValid(double w_target, double w_filler, double w_fissile) {
  // w_target must be in between w_filler and w_fissile for the cosi
  // equivalence technique to work - so we must check for this
  if (w_filler <= w_target && w_target <= w_fissile) {
    return true;
  } else if (w_fissile <= w_target && w_target <= w_filler) {
    return true;
  } else {
    return false;
  }
}

class FissConverter : public cyclus::Converter<cyclus::Material> {
 public:
  FissConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~FissConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    if (CosiValid(w_tgt, w_fill_, w_fiss_)) {
      return CosiFissileFrac(w_tgt, w_fill_, w_fiss_) * m->quantity();
    } else if (CosiValid(w_tgt, w_fiss_, w_topup_)) {
      // use fiss inventory as filler, and topup as fissile
      return CosiFillerFrac(w_tgt, w_fiss_, w_topup_) * m->quantity();
    } else {
      // don't bid at all
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

class FillConverter : public cyclus::Converter<cyclus::Material> {
 public:
  FillConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~FillConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    if (CosiValid(w_tgt, w_fill_, w_fiss_)) {
      return CosiFillerFrac(w_tgt, w_fill_, w_fiss_) * m->quantity();
    } else if (CosiValid(w_tgt, w_fiss_, w_topup_)) {
      // switched fissile inventory to filler so don't need any filler inventory
      return 0;
    } else {
      // don't bid at all
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

class TopupConverter : public cyclus::Converter<cyclus::Material> {
 public:
  TopupConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~TopupConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    if (CosiValid(w_tgt, w_fill_, w_fiss_)) {
      return 0;
    } else if (CosiValid(w_tgt, w_fiss_, w_topup_)) {
      // switched fissile inventory to filler and topup as fissile
      return CosiFissileFrac(w_tgt, w_fiss_, w_topup_) * m->quantity();
    } else {
      // don't bid at all
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

FuelFab::FuelFab(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    fill_size(0),
    fiss_size(0),
    throughput(0) {}

void FuelFab::EnterNotify() {
  cyclus::Facility::EnterNotify();

  if (fiss_commod_prefs.size() == 0) {
    for (int i = 0; i < fiss_commods.size(); i++) {
      fiss_commod_prefs.push_back(0);
    }
  }
}

std::set<cyclus::RequestPortfolio<Material>::Ptr>
FuelFab::GetMatlRequests() {
  using cyclus::RequestPortfolio;

  std::set<RequestPortfolio<Material>::Ptr> ports;

  bool exclusive = false;

  if (fiss.space() > cyclus::eps()) {
    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

    Material::Ptr m = cyclus::NewBlankMaterial(fiss.space());
    if (!fiss_recipe.empty()) {
      Composition::Ptr c = context()->GetRecipe(fiss_recipe);
      m = Material::CreateUntracked(fiss.space(), c);
    }

    for (int i = 0; i < fiss_commods.size(); i++) {
      std::string commod = fiss_commods[i];
      double pref = fiss_commod_prefs[i];

      port->AddRequest(m, this, commod, pref, exclusive);
    }

    // TODO (BUG): this needs to be a less-than constraint, but that
    // functionality doesn't exist for DRE yet.
    cyclus::CapacityConstraint<Material> cc(fiss.space());
    port->AddConstraint(cc);
    ports.insert(port);
  }

  if (fill.space() > cyclus::eps()) {
    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

    Material::Ptr m = cyclus::NewBlankMaterial(fill.space());
    if (!fill_recipe.empty()) {
      Composition::Ptr c = context()->GetRecipe(fill_recipe);
      m = Material::CreateUntracked(fill.space(), c);
    }
    port->AddRequest(m, this, fill_commod, fill_pref, exclusive);
    ports.insert(port);
  }

  if (topup.space() > cyclus::eps()) {
    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

    Material::Ptr m = cyclus::NewBlankMaterial(topup.space());
    if (!topup_recipe.empty()) {
      Composition::Ptr c = context()->GetRecipe(topup_recipe);
      m = Material::CreateUntracked(topup.space(), c);
    }
    port->AddRequest(m, this, topup_commod, topup_pref, exclusive);
    ports.insert(port);
  }

  return ports;
}

bool Contains(std::vector<std::string> vec, std::string s) {
  for (int i = 0; i < vec.size(); i++) {
    if (vec[i] == s) {
      return true;
    }
  }
  return false;
}

void FuelFab::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {

  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator trade;

  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    std::string commod = trade->first.request->commodity();
    double req_qty = trade->first.request->target()->quantity();
    Material::Ptr m = trade->second;

    // the checks of req_qty <= inventorySpace are important in circumstances
    // where fill_commod, topup_commod, and any of the fiss_commods may be the
    // same as each other.  Currently the case where topup_commod or
    // fill_commod are one or both inside fiss_commods, trades for each
    // inventory can get mixed up,
    // TODO: handle same commod case inventory discrimination more robustly.
    if (commod == fill_commod && req_qty <= fill.space()) {
      fill.Push(m);
    } else if (commod == topup_commod && req_qty <= topup.space()) {
      topup.Push(m);
    } else if (Contains(fiss_commods, commod) && req_qty <= fiss.space()) {
      fiss.Push(m);
    } else {
      throw cyclus::ValueError("cycamore::FuelFab was overmatched on requests");
    }
  }
}

std::set<cyclus::BidPortfolio<Material>::Ptr>
FuelFab::GetMatlBids(cyclus::CommodMap<Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;

  std::set<BidPortfolio<Material>::Ptr> ports;
  if (fiss.empty()) {
    return ports;
  }

  Material::Ptr m_fiss = fiss.Peek();
  double w_fiss = CosiWeight(m_fiss->comp(), spectrum);

  Material::Ptr m_fill;
  Material::Ptr m_topup;
  double w_fill = 0;
  double w_topup = 0;
  if (fill.count() > 0) {
    m_fill = fill.Peek();
    w_fill = CosiWeight(m_fill->comp(), spectrum);
  }
  if (topup.count() > 0) {
    m_topup = topup.Peek();
    w_topup = CosiWeight(m_topup->comp(), spectrum);
  }

  std::vector<cyclus::Request<Material>*>& reqs = commod_requests[outcommod];

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  for (int j = 0; j < reqs.size(); j++) {
    cyclus::Request<Material>* req = reqs[j];

    Composition::Ptr tgt = req->target()->comp();
    double w_tgt = CosiWeight(tgt, spectrum);
    double tgt_qty = req->target()->quantity();
    if (fill.count() > 0 && CosiValid(w_tgt, w_fill, w_fiss)) {
      double fiss_frac = CosiFissileFrac(w_tgt, w_fill, w_fiss);
      double fill_frac = 1 - fiss_frac;
      Material::Ptr m1 = Material::CreateUntracked(fiss_frac * tgt_qty, m_fiss->comp());
      Material::Ptr m2 = Material::CreateUntracked(fill_frac * tgt_qty, m_fill->comp());
      m1->Absorb(m2);

      bool exclusive = false;
      port->AddBid(req, m1, this, exclusive);
    } else if (fill.count() > 0 && topup.count() > 0 && CosiValid(w_tgt, w_fiss, w_topup)) {
      // only bid with topup if we have filler - otherwise we might be able to
      // meet target with filler when we get it. we should only use topup
      // when the fissile has too poor neutronics.
      double topup_frac = CosiFissileFrac(w_tgt, w_fiss, w_topup);
      double fiss_frac = 1 - topup_frac;
      Material::Ptr m1 = Material::CreateUntracked(topup_frac * tgt_qty, m_topup->comp());
      Material::Ptr m2 = Material::CreateUntracked(fiss_frac * tgt_qty, m_fiss->comp());
      m1->Absorb(m2);

      bool exclusive = false;
      port->AddBid(req, m1, this, exclusive);
    } // else can't meet the target - don't bid
  }

  cyclus::Converter<Material>::Ptr fissconv(new FillConverter(w_fill, w_fiss, w_topup, spectrum));
  cyclus::Converter<Material>::Ptr fillconv(new FissConverter(w_fill, w_fiss, w_topup, spectrum));
  cyclus::Converter<Material>::Ptr topupconv(new TopupConverter(w_fill, w_fiss, w_topup, spectrum));
  cyclus::CapacityConstraint<Material> fissc(fiss.quantity(), fissconv);
  // important! - the std::max calls prevent CapacityConstraint throwing a zero cap exception
  // TODO: write a test to check for this capacity adjustment
  cyclus::CapacityConstraint<Material> fillc(std::max(fill.quantity(), 1e-100), fillconv);
  cyclus::CapacityConstraint<Material> topupc(std::max(topup.quantity(), 1e-100), topupconv);
  port->AddConstraint(fillc);
  port->AddConstraint(fissc);
  port->AddConstraint(topupc);

  cyclus::CapacityConstraint<Material> cc(throughput);
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void FuelFab::GetMatlTrades(
    const std::vector< cyclus::Trade<Material> >& trades,
    std::vector<std::pair<cyclus::Trade<Material>,
    Material::Ptr> >& responses) {
  using cyclus::Trade;

  Material::Ptr m_fiss = fiss.Peek();
  double w_fiss = CosiWeight(m_fiss->comp(), spectrum);

  Material::Ptr m_fill = fill.Peek();
  double w_fill = CosiWeight(m_fill->comp(), spectrum);

  Material::Ptr m_topup;
  double w_topup = 0;
  if (topup.count() > 0) {
    m_topup = topup.Peek();
    w_topup = CosiWeight(m_topup->comp(), spectrum);
  }

  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (int i = 0; i < trades.size(); i++) {
    Material::Ptr tgt = trades[i].request->target();
    double w_tgt = CosiWeight(tgt->comp(), spectrum);
    double qty = tgt->quantity();

    if (CosiValid(w_tgt, w_fill, w_fiss)) {
      double fiss_frac = CosiFissileFrac(w_tgt, w_fill, w_fiss);
      double fill_frac = 1 - fiss_frac;

      Material::Ptr m = fiss.Pop(fiss_frac*qty);
      m->Absorb(fill.Pop(fill_frac*qty));
      responses.push_back(std::make_pair(trades[i], m));
    } else {
      double topup_frac = CosiFissileFrac(w_tgt, w_fiss, w_topup);
      double fiss_frac = 1 - topup_frac;

      Material::Ptr m = topup.Pop(topup_frac*qty);
      m->Absorb(fiss.Pop(fiss_frac*qty));
      responses.push_back(std::make_pair(trades[i], m));
    }
  }
}

extern "C" cyclus::Agent* ConstructFuelFab(cyclus::Context* ctx) {
  return new FuelFab(ctx);
}

} // namespace cycamore

