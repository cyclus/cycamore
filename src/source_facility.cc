#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "source_facility.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility(cyclus::Context* ctx)
  : cyclus::Facility(ctx),
    out_commod(""),
    recipe_name(""),
    capacity(std::numeric_limits<double>::max()) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::~SourceFacility() {}

#pragma cyclus def clone cycamore::SourceFacility

#pragma cyclus def schema cycamore::SourceFacility

#pragma cyclus def annotations cycamore::SourceFacility

#pragma cyclus def infiletodb cycamore::SourceFacility

#pragma cyclus def snapshot cycamore::SourceFacility

#pragma cyclus def snapshotinv cycamore::SourceFacility

#pragma cyclus def initinv cycamore::SourceFacility

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::InitFrom(SourceFacility* m) {
  #pragma cyclus impl initfromcopy cycamore::SourceFacility
  cyclus::toolkit::CommodityProducer::Copy(m);
}

void SourceFacility::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb cycamore::SourceFacility

  cyclus::toolkit::Commodity commod(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyclus::Facility::str()
     << " supplies commodity '"
     << out_commod << "' with recipe '"
     << recipe_name << "' at a capacity of "
     << capacity << " kg per time step ";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::Tick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << prototype() << " is ticking {";
  LOG(cyclus::LEV_INFO4, "SrcFac") << "will offer " << capacity
                                   << " kg of "
                                   << out_commod << ".";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
  current_capacity = capacity; // reset capacity
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::Tock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << prototype() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::EnterNotify() {
  cyclus::CommodityProducer* cast =
      dynamic_cast<cyclus::CommodityProducerManager*>(parent());
  if (cast != NULL)
    parent()->Register(this);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr SourceFacility::GetOffer(
    const cyclus::Material::Ptr target) const {
  using cyclus::Material;
  double qty = std::min(target->quantity(), capacity);
  return Material::CreateUntracked(qty, context()->GetRecipe(recipe_name));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
SourceFacility::GetMatlBids(
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
void SourceFacility::GetMatlTrades(
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
extern "C" cyclus::Agent* ConstructSourceFacility(cyclus::Context* ctx) {
  return new SourceFacility(ctx);
}

} // namespace cycamore
