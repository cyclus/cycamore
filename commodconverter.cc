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
     << "     Input Commodity = " << in_commod_() << ",\n"
     << "     Output Commodity = " << out_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Capacity = " << capacity_() << ",\n"
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
  while( processing[ready()].count() > 0 ) {
    Convert_(); // place processing into stocks
  }
  BeginProcessing_(); // place unprocessed inventory into processing
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

    port->AddRequest(mat, this, in_commod_());

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
CommodconverterFacility::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::set<std::string>::const_iterator it;
  BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                              out_commod_(),
                                              &stocks);
  if (!port->bids().empty()) {
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

  // for each trade, respond
  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = it->amt;
    // create a material pointer representing what you can offer
    Material::Ptr response = TradeResponse_(qty, &stocks);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << commodity;
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
                                << " of " << in_commod_()
                                << " to its inventory, which is holding "
                                << inventory.quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr CommodconverterFacility::Request_() {
  double qty = std::max(0.0, current_capacity());
  return cyclus::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr CommodconverterFacility::GetBids_(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests,
    std::string commod,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Composition;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  if (commod_requests.count(commod) > 0 && buffer->quantity() > 0) {
    std::vector<Request<Material>*>& requests = commod_requests.at(commod);

    // get offer composition
    Material::Ptr back = ResCast<Material>(buffer->Pop(ResourceBuff::BACK));
    Composition::Ptr comp = back->comp();
    buffer->Push(back);

    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      double qty = std::min(req->target()->quantity(), buffer->quantity());
      Material::Ptr offer = Material::CreateUntracked(qty, comp);
      port->AddBid(req, offer, this);
    }

    CapacityConstraint<Material> cc(buffer->quantity());
    port->AddConstraint(cc);
  }

  return port;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr CommodconverterFacility::TradeResponse_(
    double qty,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from inventory and blob it into one material
    manifest = ResCast<Material>(buffer->PopQty(qty));
  } catch(cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  Material::Ptr response = manifest[0];
  crctx_.RemoveRsrc(response);
  for (int i = 1; i < manifest.size(); i++) {
    crctx_.RemoveRsrc(manifest[i]);
    response->Absorb(manifest[i]);
  }
  return response;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::BeginProcessing_(){
  LOG(cyclus::LEV_DEBUG2, "ComCnv") << "CommodConverter " << prototype() 
                                    << " added resources to processing";
  if( inventory.count() > 0 ){
    try {
      processing[context()->time()].Push(inventory.Pop());
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::Convert_(){
  using cyclus::Material;
  using cyclus::ResCast;
  LOG(cyclus::LEV_DEBUG2, "ComCnv") << "CommodConverter " << prototype() 
                                    << " removed a resource from processing.";

  if ( processing.find(ready())->second.count() > 0 ) {
    try {
      // pop one material from processing 
      Material::Ptr mat = ResCast<Material>(processing.find(ready())->second.Pop());
      // change its commod
      crctx_.UpdateRsrc(out_commod_(), mat);
      // put it in the stocks
      stocks.Push(mat);
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructCommodconverterFacility(cyclus::Context* ctx) {
  return new CommodconverterFacility(ctx);
}

} // namespace commodconverter
