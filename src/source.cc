#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "source.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Source::Source(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    out_commod(""),
    recipe_name(""),
    capacity(std::numeric_limits<double>::max()) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Source::~Source() {}

#pragma cyclus def clone cycamore::Source

#pragma cyclus def schema cycamore::Source

#pragma cyclus def annotations cycamore::Source

#pragma cyclus def infiletodb cycamore::Source

#pragma cyclus def snapshot cycamore::Source

#pragma cyclus def snapshotinv cycamore::Source

#pragma cyclus def initinv cycamore::Source

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy cycamore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb cycamore::Source

  commod_ = cyclus::toolkit::Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod_);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod_, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod_, capacity);
}

void Source::EnterNotify() {
  Facility::EnterNotify();
  commod_ = cyclus::toolkit::Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod_);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod_, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod_, capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Source::str() {
  std::stringstream ss;
  std::string ans = std::string(cyclus::toolkit::CommodityProducer::Produces(cyclus::toolkit::Commodity(out_commod)) ? "yes" : "no");
  ss << cyclus::Facility::str()
     << " supplies commodity '"
     << out_commod << "' with recipe '"
     << recipe_name << "' at a capacity of "
     << capacity << " kg per time step "
     << " commod producer members: "
                                                                 << " produces " << out_commod << "?: " <<  ans
     << " capacity: " << cyclus::toolkit::CommodityProducer::Capacity(commod_)
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(commod_);
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::Tick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << prototype() << " is ticking {";
  LOG(cyclus::LEV_INFO4, "SrcFac") << "will offer " << capacity
                                   << " kg of "
                                   << out_commod << ".";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "Stats: " << str();
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
  current_capacity = capacity; // reset capacity
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::Tock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << prototype() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr Source::GetOffer(
    const cyclus::Material::Ptr target) const {
  using cyclus::Material;
  double qty = std::min(target->quantity(), capacity);
  return Material::CreateUntracked(qty, context()->GetRecipe(recipe_name));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
Source::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Request;
  
  std::set<BidPortfolio<Material>::Ptr> ports;
  
  if (commod_requests.count(out_commod) > 0) {
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
    std::vector<Request<Material>*>& requests =
        commod_requests[out_commod];

    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      Material::Ptr offer = GetOffer(req->target());
      port->AddBid(req, offer, this);
    }

    CapacityConstraint<Material> cc(capacity);
    port->AddConstraint(cc);
    ports.insert(port);
  }
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Source::GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  double provided = 0;
  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    current_capacity -= qty;
    provided += qty;
    // @TODO we need a policy on negatives..
    Material::Ptr response = Material::Create(this,
                                              qty,
                                              context()->GetRecipe(recipe_name));
    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "SrcFac") << prototype() << " just received an order"
                                     << " for " << qty
                                     << " of " << out_commod;
  }
  if (cyclus::IsNegative(current_capacity)) {
    std::stringstream ss;
    ss << "is being asked to provide " << provided
       << " but its capacity is " << capacity << "."; 
    throw cyclus::ValueError(Agent::InformErrorMsg(ss.str()));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

} // namespace cycamore

// required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif // CYCLUS_AGENT_TESTS_CONNECTED
