#include "source.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

namespace cycamore {

Source::Source(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      throughput(std::numeric_limits<double>::max()),
      inventory_size(std::numeric_limits<double>::max()),
      latitude(0.0),
      longitude(0.0),
      package(cyclus::Package::unpackaged_name()),
      transport_unit(cyclus::TransportUnit::unrestricted_name()),
      coordinates(latitude, longitude) {}

Source::~Source() {}

void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy cycamore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb cycamore::Source
  namespace tk = cyclus::toolkit;
  tk::CommodityProducer::Add(tk::Commodity(outcommod),
                             tk::CommodInfo(throughput, throughput));
}

std::string Source::str() {
  namespace tk = cyclus::toolkit;
  std::stringstream ss;
  std::string ans;
  if (cyclus::toolkit::CommodityProducer::Produces(
          cyclus::toolkit::Commodity(outcommod))) {
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str() << " supplies commodity '" << outcommod
     << "' with recipe '" << outrecipe << "' at a throughput of "
     << throughput << " kg per time step "
     << " commod producer members: "
     << " produces " << outcommod << "?: " << ans
     << " throughput: " << cyclus::toolkit::CommodityProducer::Capacity(outcommod)
     << " with package type: " << package
     << " and transport unit type: " << transport_unit
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(outcommod);
  return ss.str();
}

void Source::EnterNotify() {
  using cyclus::CompMap;
  using cyclus::Composition;
  using cyclus::Material;
  cyclus::Facility::EnterNotify();
  RecordPosition();

  // create all source inventory and place into buf
  cyclus::Material::Ptr all_inv;
  Composition::Ptr blank_comp = Composition::CreateFromMass(CompMap());

  all_inv = (outrecipe.empty() || context() == NULL) ? \
          Material::Create(this, inventory_size, blank_comp) : \
          Material::Create(this, inventory_size, context()->GetRecipe(outrecipe));
  inventory.Push(all_inv);
}

std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> Source::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Package;
  using cyclus::Request;
  using cyclus::TransportUnit;

  double max_qty = std::min(throughput, inventory.quantity());
  cyclus::toolkit::RecordTimeSeries<double>("supply"+outcommod, this,
                                            max_qty);
  LOG(cyclus::LEV_INFO3, "Source") << prototype() << " is bidding up to "
                                   << max_qty << " kg of " << outcommod;
  LOG(cyclus::LEV_INFO5, "Source") << "stats: " << str();

  std::set<BidPortfolio<Material>::Ptr> ports;
  if (max_qty < cyclus::eps()) {
    return ports;
  } else if (commod_requests.count(outcommod) == 0) {
    return ports;
  }

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  std::vector<Request<Material>*>& requests = commod_requests[outcommod];
  std::vector<Request<Material>*>::iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    Request<Material>* req = *it;
    Material::Ptr target = req->target();
    double qty = std::min(target->quantity(), max_qty);

    // calculate packaging
    std::pair<double, int> fill = context()->GetPackage(package)->GetFillMass(qty);
    double bid_qty = fill.first;
    int n_full_bids = fill.second;
    Package::ExceedsSplitLimits(n_full_bids);

    std::vector<double> bids;
    bids.assign(n_full_bids, bid_qty);

    double remaining_qty = qty - (n_full_bids * bid_qty);
    if ((remaining_qty > cyclus::eps()) && (remaining_qty >= context()->GetPackage(package)->fill_min())) {
      bids.push_back(remaining_qty);
    }

    // calculate transport units
    int shippable_pkgs = context()->GetTransportUnit(transport_unit)
                         ->MaxShippablePackages(bids.size());
    if (shippable_pkgs < bids.size()) {
      bids.erase(bids.begin() + shippable_pkgs, bids.end());
    }

    std::vector<double>::iterator bit;
    for (bit = bids.begin(); bit != bids.end(); ++bit) {
      Material::Ptr m;
      m = outrecipe.empty() ? \
          Material::CreateUntracked(*bit, target->comp()) : \
          Material::CreateUntracked(*bit, context()->GetRecipe(outrecipe));
      port->AddBid(req, m, this);
    }
  }

  CapacityConstraint<Material> cc(max_qty);
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

void Source::GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  int shippable_trades = context()->GetTransportUnit(transport_unit)
                         ->MaxShippablePackages(trades.size());

  std::vector<Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    if (shippable_trades > 0) {
      double qty = it->amt;

      Material::Ptr m = inventory.Pop(qty);
      
      std::vector<Material::Ptr> m_pkgd = m->Package<Material>(context()->GetPackage(package));

      if (m->quantity() > cyclus::eps()) {
        // If not all material is packaged successfully, return the excess
        // amount to the inventory
        inventory.Push(m);
      }

      Material::Ptr response;
      if (m_pkgd.size() > 0) {
        // Because we responded (in GetMatlBids) with individual package-sized
        // bids, each packaged vector is guaranteed to have no more than one
        // package in it. This single packaged resource is our response
        response = m_pkgd[0];
        shippable_trades -= 1;
      }

      if (outrecipe.empty() && response->comp() != it->request->target()->comp()) {
        response->Transmute(it->request->target()->comp());
      }

      responses.push_back(std::make_pair(*it, response));
      LOG(cyclus::LEV_INFO5, "Source") << prototype() << " sent an order"
                                      << " for " << response->quantity() << " of " << outcommod;
    }
  }
}

void Source::RecordPosition() {
  std::string specification = this->spec();
  context()
      ->NewDatum("AgentPosition")
      ->AddVal("Spec", specification)
      ->AddVal("Prototype", this->prototype())
      ->AddVal("AgentId", id())
      ->AddVal("Latitude", latitude)
      ->AddVal("Longitude", longitude)
      ->Record();
}

extern "C" cyclus::Agent* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

}  // namespace cycamore
