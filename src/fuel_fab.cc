#include "recipe_mixer.h"
#include "kitlus/fuel_match.h"

#define LG(X) LOG(cyclus::LEV_##X, "RecMix")

using cyclus::Material;
using cyclus::Composition;
using cyclus::ResCast;

namespace cycamore {

FuelFab::FuelFab(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    fill_size_(0),
    fiss_size_(0),
    throughput_(0) {}

void FuelFab::EnterNotify() {
  cyclus::Facility::EnterNotify();

  fillpolicy_.Init(this, &fill_, "filler")
             .Set(fill_commod_, context()->GetRecipe(fill_recipe_))
             .Start();
  fisspolicy_.Init(this, &fiss_, "fissile")
             .Set(fiss_commod_, context()->GetRecipe(fiss_recipe_))
             .Start();
}

void FuelFab::Tick() {
  LG(INFO3) << "FuelFab id=" << id() << " is ticking";
  LG(INFO4) << "filler quantity = " << fill_.quantity();
  LG(INFO4) << "fissile quantity = " << fiss_.quantity();
  LG(INFO4) << "outbuf quantity = " << out_.quantity();
  double qty = std::min(throughput_, out_.space());
  if (fill_.quantity() < cyclus::eps() || fiss_.quantity() < cyclus::eps() || qty < cyclus::eps()) {
    return;
  }

  Material::Ptr m_fill = fill.Push(Squash(fill.PopN(fill.count())));
  Material::Ptr m_fiss = fiss.Push(Squash(fiss.PopN(fiss.count())));

  // determine frac needed from each input stream
  Composition::Ptr tgt = context()->GetRecipe(outrecipe_);
  double frac2 = kitlus::CosiFissileFrac(tgt, m1->comp(), m2->comp());
  double frac1 = 1 - frac2;
  if (frac2 < 0) {
    fill_.Push(m1);
    fiss_.Push(m2);
    LG(ERROR) << "fiss stream has too low reactivity";
    return;
  }

  LG(INFO4) << "fill frac = " << frac1;
  LG(INFO4) << "fiss frac = " << frac2;

  // deal with stream quantity and out buf space constraints
  double qty1 = frac1 * qty;
  double qty2 = frac2 * qty;
  double qty1diff = m1->quantity() - qty1;
  double qty2diff = m2->quantity() - qty2;
  if (qty1diff >= 0 && qty2diff >= 0) {
    // not constrained by inbuf quantities
  } else if (qty1diff < qty2diff ) {
    // constrained by fiss_
    LG(INFO5) << "Constrained by incommod '" << fill_commod_
              << "' - reducing qty from " << qty
              << " to " << m1->quantity() / frac1;
    qty = m1->quantity() / frac1;
  } else {
    // constrained by inbuf2
    LG(INFO5) << "Constrained by incommod '" << fiss_commod_
              << "' - reducing qty from " << qty
              << " to " << m2->quantity() / frac2;
    qty = m2->quantity() / frac2;
  }

  Material::Ptr mix = m1->ExtractQty(std::min(frac1 * qty, m1->quantity()));
  mix->Absorb(m2->ExtractQty(std::min(frac2 * qty, m2->quantity())));

  cyclus::toolkit::MatQuery mq(mix);
  LG(INFO4) << "Mixed " << mix->quantity() << " kg to recipe";
  LG(INFO5) << " u238 = " << mq.mass_frac(922380000);
  LG(INFO5) << " u235 = " << mq.mass_frac(922350000);
  LG(INFO5) << "Pu239 = " << mq.mass_frac(942390000);

  out_.Push(mix);
  if (m1->quantity() > 0) {
    fill_.Push(m1);
  }
  if (m2->quantity() > 0) {
    fiss_.Push(m2);
  }
}

std::set<cyclus::BidPortfolio<Material>::Ptr>
FuelFab::GetMatlBids(cyclus::CommodMap<Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;

  std::set<BidPortfolio<Material>::Ptr> ports;

  Material::Ptr m_fill = Squash(fill.PopN(fill.count()));
  Material::Ptr m_fiss = Squash(fiss.PopN(fiss.count()));
  Material::Ptr m_topup = Squash(topup.PopN(topup.count()));
  topup.Push(m_topup);
  fiss.Push(m_fiss);
  fill.Push(m_fill);

  std::vector<Request<Material>*>& reqs = commod_requests[outcommod];

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

  double w_fill = CosiWeight(m_fill->comp(), spectrum);
  double w_fiss = CosiWeight(m_fiss->comp(), spectrum);
  double w_topup = CosiWeight(m_topup->comp(), spectrum);
  Converter<Material>::Ptr fillconv(new FissConverter(w_fill, w_fiss, w_topup, spectrum));
  Converter<Material>::Ptr fissconv(new FillConverter(w_fill, w_fiss, w_topup, spectrum));
  Converter<Material>::Ptr topupconv(new TopupConverter(w_fill, w_fiss, w_topup, spectrum));
  CapacityConstraint<Material> fillc(fill.quantity(), fillconv);
  CapacityConstraint<Material> fissc(fiss.quantity(), fissconv);
  CapacityConstraint<Material> topupc(topup.quantity(), topupconv);
  port->AddConstraint(fillc);
  port->AddConstraint(fissc);
  port->AddConstraint(topupc);

  cyclus::CapacityConstraint<Material> cc(throughput);
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

} // namespace cycamore

extern "C" cyclus::Agent* ConstructFuelFab(cyclus::Context* ctx) {
  return new FuelFab(ctx);
}



