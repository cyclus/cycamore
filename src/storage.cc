// storage.cc
// Implements the Storage class
#include "storage.h"

namespace storage {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Storage::Storage(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("The Storage Facility is experimental.");
    };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema storage::Storage

#pragma cyclus def annotations storage::Storage

#pragma cyclus def initinv storage::Storage

#pragma cyclus def snapshotinv storage::Storage

#pragma cyclus def infiletodb storage::Storage

#pragma cyclus def snapshot storage::Storage

#pragma cyclus def clone storage::Storage

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::InitFrom(Storage* m) {

  #pragma cyclus impl initfromcopy storage::Storage

  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::InitFrom(cyclus::QueryableBackend* b){

  #pragma cyclus impl initfromdb storage::Storage

  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::EnterNotify() {
  Facility::EnterNotify();

  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Storage::str() {
  std::stringstream ss;
  std::string ans;
  if (cyclus::toolkit::CommodityProducer::
      Produces(cyclus::toolkit::Commodity(out_commod_()))){
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodity = " << in_commod_() << ",\n"
     << "     Output Commodity = " << out_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Capacity = " << capacity_() << ",\n"
     << " commod producer members: " << " produces "
     << out_commod << "?:" << ans
     << " capacity: " << cyclus::toolkit::CommodityProducer::Capacity(out_commod_())
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(out_commod_())
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tick() { 
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is ticking {";

  if (current_capacity() > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "ComCnv") << " has capacity for " << current_capacity()
                                       << " kg of " << in_commod
                                       << " recipe: " << in_recipe << ".";
  }

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tock() {
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is tocking {";

  BeginProcessing_(); // place unprocessed inventory into processing

  if( ready() >= 0 || process_time_() == 0 ) {
    ProcessMat_(capacity_()); // place processing into stocks
  }

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
Storage::GetMatlRequests() {
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
void Storage::AcceptMatlTrades(
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
Storage::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::set<std::string>::const_iterator it;
  BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                              out_commod,
                                              &stocks);
  if (!port->bids().empty()) {
    ports.insert(port);
  }

  return ports;

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  // for each trade, respond
  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = stocks.quantity();
    // create a material pointer representing what you can offer
    if ( qty > 0 ) {
      Material::Ptr response = TradeResponse_(qty, &stocks);
      responses.push_back(std::make_pair(*it, response));
    }
    LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << commodity;
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::AddMat_(cyclus::Material::Ptr mat) {
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
cyclus::Material::Ptr Storage::Request_() {
  double qty = std::max(0.0, current_capacity());
  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " just requested "
                                  << current_capacity()
                                  << " of commodity: " << in_commod
                                  << " with recipe: " << in_recipe;
  return cyclus::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr Storage::GetBids_(
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
cyclus::Material::Ptr Storage::TradeResponse_(
    double qty,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from inventory and blob it into one material
    manifest = ResCast<Material>(buffer->PopQty(qty));
    Material::Ptr response = manifest[0];
    for (int i = 1; i < manifest.size(); i++) {
      response->Absorb(manifest[i]);
    }
    return response;
  } catch(cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::BeginProcessing_(){
  while( inventory.count() > 0 ){
    try {
      processing[context()->time()].Push(inventory.Pop());
      LOG(cyclus::LEV_DEBUG2, "ComCnv") << "Storage " << prototype() 
                                      << " added resources to processing at t= "
                                      << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::ProcessMat_(double cap){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;
  using cyclus::toolkit::Manifest;

  int t = ready();

  if ( ProcessingAmt_(t) > 0 ){
    try {
      double to_pop = std::min(cap, processing[t].quantity());

      // pop appropriate amount of material from processing 
      std::vector<Material::Ptr> to_conv = 
        ResCast<Material>(processing[t].PopQty(to_pop));
      // Push process ready material into stocks
        stocks.PushAll(to_conv);

      AdvanceUnconverted_(t);
      LOG(cyclus::LEV_INFO1, "ComCnv") << "Storage " << prototype() 
                                        << " converted quantity : " << to_pop 
                                        << " from " << in_commod 
                                        << " to " << out_commod
                                        << " at t= " << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Storage::ProcessingAmt_(int time) {
  using cyclus::toolkit::ResourceBuff;
  double to_ret = 0;
  std::map<int, ResourceBuff>::iterator proc = processing.find(time);
  if ( proc!=processing.end() && 
      proc->second.quantity() > 0 ) {
    to_ret = proc->second.quantity();
  }
  return to_ret;

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::AdvanceUnconverted_(int time){
  using cyclus::Material;
  using cyclus::ResCast;

  double to_pop = ProcessingAmt_(time);
  if ( to_pop > 0 ) {
    try {
      std::vector<Material::Ptr> to_advance = 
      ResCast<Material>(processing[time].PopQty(to_pop));
      processing[time+1].PushAll(to_advance);
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStorage(cyclus::Context* ctx) {
  return new Storage(ctx);
}

} // namespace storage