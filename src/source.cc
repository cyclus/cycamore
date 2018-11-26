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
      coordinates(latitude, longitude) {}

Source::~Source() {}

void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy cycamore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
  RecordPosition();
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb cycamore::Source
  namespace tk = cyclus::toolkit;
  tk::CommodityProducer::Add(tk::Commodity(outcommod),
                             tk::CommodInfo(throughput, throughput));
  RecordPosition();

}

void Source::Tick() {
  using cyclus::Material;
  using cyclus::toolkit::ResBuf;
  // push amount of throughput to ready buffer
  cyclus::CompMap v;
  cyclus::Composition::Ptr comp = cyclus::Composition::CreateFromAtom(v);
  if (!outrecipe.empty()){
    comp = context()->GetRecipe(outrecipe);
  }
  double qty = std::min(throughput, inventory_size);
  Material::Ptr m = Material::Create(this, qty, comp);
  inventory_size -= qty;
  if (buffer == false && !ready.empty()){
    ready.Pop();
  }
  ready.Push(m);
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
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(outcommod);
  return ss.str();
}

std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> Source::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::toolkit::ResBuf;

  double max_qty = ready.quantity();
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
    Material::Ptr m = Material::CreateUntracked(qty, target->comp());
    if (!outrecipe.empty()) {
      m = Material::CreateUntracked(qty, context()->GetRecipe(outrecipe));
    }
    port->AddBid(req, m, this);
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
  using cyclus::toolkit::ResBuf;

  std::vector<cyclus::Trade<cyclus::Material> >::const_iterator it;

  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    Material::Ptr response;
    response = ready.Pop(qty);
    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "Source") << prototype() << " sent an order"
                                     << " for " << qty << " of " << outcommod;
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
