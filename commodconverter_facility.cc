// commodconverter_facility.cc
// Implements the CommodconverterFacility class
#include "commodconverter_facility.h"

namespace commodconverter {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterFacility::CommodconverterFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema commodconverter::CommodconverterFacility

#pragma cyclus def annotations commodconverter::CommodconverterFacility

#pragma cyclus def initinv commodconverter::CommodconverterFacility

#pragma cyclus def snapshotinv commodconverter::CommodconverterFacility

#pragma cyclus def initfromdb commodconverter::CommodconverterFacility

#pragma cyclus def initfromcopy commodconverter::CommodconverterFacility

#pragma cyclus def infiletodb commodconverter::CommodconverterFacility

#pragma cyclus def snapshot commodconverter::CommodconverterFacility

#pragma cyclus def clone commodconverter::CommodconverterFacility

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string CommodconverterFacility::str() {
  std::stringstream ss;
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodity = " << in_commod() << ",\n"
     << "     Output Commodity = " << out_commod() << ",\n"
     << "     Process Time = " << process_time() << ",\n"
     << "     Capacity = " << capacity() << ",\n"
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::Tick() { 
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is ticking {";
  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::Tock() {
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
CommodconverterFacility::GetMatlRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat = Request_();
  double amt = mat->quantity();

  if (amt > cyclus::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    port->AddRequest(mat, this, in_commod());

    ports.insert(port);
  }

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::AcceptMatlTrades(
  const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    AddMat_(it->second);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
CommodconverterFacility::GetMatlBids(
  cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;

  std::set<BidPortfolio<Material>::Ptr> ports;

  if (commod_requests.count(out_commod()) > 0 && inventory.quantity() > 0) {
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    std::vector<Request<Material>*>& requests =
        commod_requests[out_commod()];

    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      Material::Ptr offer = Offer_(req->target());
      port->AddBid(req, offer, this);
    }

    // want to add constraints based on the capacity for the incoming commodity
    CapacityConstraint<Material> avail(current_capacity());
    port->AddConstraint(avail);

    LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " adding a capacity constraint of "
                                  << avail.capacity();

    ports.insert(port);
  }
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = it->amt;
    Material::Ptr response = TradeResponse_(qty, &storage_[commodity]);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << out_commod();
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::AddMat_(cyclus::Material::Ptr mat) {
  // Here we do not check that the recipe matches the input recipe. 

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " is initially holding "
                                << inventory.quantity() << " total.";

  try {
    inventory.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " added " << mat->quantity()
                                << " of " << in_commod()
                                << " to its inventory, which is holding "
                                << inventory.quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr CommodconverterFacility::Request_() {
  double qty = std::max(0.0, MaxInventorySize() - InventorySize());
  return cyclus::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructCommodconverterFacility(cyclus::Context* ctx) {
  return new CommodconverterFacility(ctx);
}

} // namespace commodconverter
