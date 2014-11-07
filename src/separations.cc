#include "separations.h"

namespace cycamore {

Separations::Separations(cyclus::Context* ctx) : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Separations facility " \
                                             "is experimental.");
};

std::string Separations::str() {
  std::stringstream ss;
  std::string ans;
  std::stringstream prod;

  std::vector<std::string>::const_iterator it;
  for(it = out_commods.begin(); it != out_commods.end(); ++it ) {
    if (cyclus::toolkit::CommodityProducer::
        Produces(cyclus::toolkit::Commodity(*it))){
      ans = "yes";
    } else {
      ans = "no";
    }
    prod << *it << "?:" << ans
         << " capacity: " << cyclus::toolkit::CommodityProducer::Capacity(*it)
         << " cost: " << cyclus::toolkit::CommodityProducer::Cost(*it);
  }


  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodity = " << in_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Maximum Inventory Size = " << max_inv_size_() << ",\n"
     << "     Capacity = " << capacity_() << ",\n"
     << "     Current Capacity = " << CurrentCapacity() << ",\n"
     << "     Cost = " << cost_() << ",\n"
     << "     Separated Quantity = " << SepbuffQuantity() << ",\n"
     << "     Raw Quantity = " << rawbuff_.quantity() << ",\n"
     << " commod producer members: " << " produces "
     << prod.str()
     << "'}";
  return ss.str();
}

void Separations::Tick() {
  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "Separartions") << prototype() << " is ticking at time "
                                         << time << " {";
  LOG(cyclus::LEV_DEBUG4, "Separartions") << "Current facility parameters for "
                                          << prototype()
                                          << " at the beginning of the tick are:";
  PrintStatus();
  // if lifetime is up, clear self of materials??
  double currcap = CurrentCapacity();
  if (currcap > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "Separartions") << " will request " << currcap
                                           << " kg of " << in_commod << ".";
  }
}

std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> 
    Separations::GetMatlRequests() {
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  double amt = CurrentCapacity();
  if (amt < cyclus::eps()) {
    return ports;
  }

  std::vector<Request<Material>*> mutuals;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  for (int i = 0; i < in_commods.size(); i++) {
    Material::Ptr m = cyclus::NewBlankMaterial(amt);
    mutuals.push_back(port->AddRequest(m, this, in_commods[i]));
  }

  port->AddMutualReqs(mutuals);
  ports.insert(port);
  return ports;
}

std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> Separations::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::vector<std::string>::const_iterator it;
  for (it = out_commods.begin(); it != out_commods.end(); ++it) {
    BidPortfolio<Material>::Ptr port = GetBids(commod_requests, *it,
                                                &sepbuff_[*it]);
    if (!port->bids().empty()) {
      ports.insert(port);
    }
  }
  return ports;
}

cyclus::BidPortfolio<cyclus::Material>::Ptr Separations::GetBids(
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
    std::vector<Request<Material>*>& requests = commod_requests[commod];

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
void Separations::GetMatlTrades(
    const std::vector<cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  // for each trade, respond
  std::vector<Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = std::min(it->amt, SepbuffQuantity(commodity));
    // create a material pointer representing what you can offer
    if (qty > 0.0) {
      Material::Ptr response = TradeResponse(qty, &sepbuff_[commodity]);
      responses.push_back(std::make_pair(*it, response));
    }
    LOG(cyclus::LEV_INFO5, "Separations") << prototype()
                                          << " just received an order"
                                          << " for " << it->amt
                                          << " of " << commodity;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::AcceptMatlTrades(
    const std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                                cyclus::Material::Ptr> >& responses) {
  // accept blindly, no judgement, any material that has been matched
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> >::const_iterator it;
  // put each in rawbuff 
  for (it = responses.begin(); it != responses.end(); ++it) {
    AddMat(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Tock() {
  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "Separartions") << prototype() << " is tocking {";
  LOG(cyclus::LEV_DEBUG4, "Separartions") << "Current facility parameters for "
                                          << prototype()
                                          << " at the beginning of the tock are:";
  PrintStatus();
  BeginProcessing();
  if(ready() >= 0){
    Separate();
  }
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tock are:";
  PrintStatus();
  LOG(cyclus::LEV_INFO3, "Separartions") << "}";

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::AddMat(cyclus::Material::Ptr mat) {
  // Here we do not check that the recipe matches the input recipe.
  LOG(cyclus::LEV_INFO5, "Separartions") << prototype() << " is initially holding "
                                         << rawbuff_.quantity() << " total.";
  try {
    rawbuff_.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }
  LOG(cyclus::LEV_INFO5, "Separartions") << prototype() << " added " 
                                         << mat->quantity()
                                         << " of " << in_commod_()
                                         << " to its rawbuff, which is holding "
                                         << rawbuff_.quantity() << " total.";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::PrintStatus() {
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     ProcessTime: " << process_time_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Input Commodity = " << in_commod_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Process Time = " << process_time_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Maximum Inventory Size = " << max_inv_size_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Capacity = " << capacity_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Current Capacity = " << CurrentCapacity();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Cost = " << cost_();
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Separated Quantity = " << SepbuffQuantity() ;
  LOG(cyclus::LEV_DEBUG3, "Separartions") << "     Raw Quantity = " << rawbuff_.quantity() ;
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr Separations::TradeResponse(
    double qty,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from rawbuff and blob it into one material
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

const double Separations::SepbuffQuantity() const {
  using cyclus::toolkit::ResourceBuff;
  double total = 0.0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for(it = sepbuff_.begin(); it != sepbuff_.end(); ++it) {
    total += (*it).second.quantity();
  }
  return total;
}

const double Separations::SepbuffQuantity(std::string commod) const {
  using cyclus::toolkit::ResourceBuff;
  std::map<std::string, ResourceBuff>::const_iterator found;
  found = sepbuff_.find(commod);
  double amt = 0.0;
  if (found != sepbuff_.end()){
    amt = (*found).second.quantity();
  } 
  return amt;
}

void Separations::Separate(){
  if (processing.find(ready()) != processing.end()) {
    Separate(&processing[ready()]);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Separate(cyclus::toolkit::ResourceBuff* buff){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  while (!buff->empty()){
    Material::Ptr back = ResCast<Material>(buff->Pop(ResourceBuff::BACK));
    Separate(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Separate(cyclus::Material::Ptr mat){
  using cyclus::CompMap;
  using cyclus::Composition;
  using std::make_pair;
  using std::string;

  std::map<string, CompMap> sep_comps;
  CompMap::iterator entry;
  CompMap orig = mat->comp()->mass();
  double tot = mat->quantity();
  for (entry = orig.begin(); entry != orig.end(); ++entry){
    int iso = int(entry->first);
    int elem = int(iso/10000000.); // convert iso to element
    double sep = entry->second*tot*Eff(elem); // access matrix
    string stream = Stream(elem);
    sep_comps[stream][iso] = sep;
  }
  std::map< string, CompMap >::iterator str;
  for(str=sep_comps.begin(); str!=sep_comps.end(); ++str){
    CompMap to_extract = (*str).second;
    double qty = cyclus::compmath::Sum(to_extract);
    Composition::Ptr c = Composition::CreateFromMass(to_extract);
    sepbuff_[(*str).first].Push(mat->ExtractComp(qty, c));
    LOG(cyclus::LEV_DEBUG3, "Separartions") << "Separations " << prototype()
                                      << " separated quantity : "
                                      << qty << " at t = "
                                      << context()->time();
  }
  sepbuff_[waste_stream].Push(mat);
  LOG(cyclus::LEV_DEBUG2, "Separartions") << "Separations " << prototype()
                                    << " separated material at t = "
                                    << context()->time();
  LOG(cyclus::LEV_DEBUG2, "Separartions") << "Separations " << prototype()
                                    << " now has a separated quantity : "
                                    << SepbuffQuantity()
                                    << " and a raw quantity : "
                                    << rawbuff_.quantity();

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Separations::ElemIdx(int element){
  int to_ret = find(elems.begin(), elems.end(), element) - elems.begin();

  if( to_ret > elems.size() ){
    throw cyclus::KeyError("The element was not found in the matrix");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Separations::Eff(int element){
  double to_ret = 0;
  try {
    int idx = ElemIdx(element);
    if ( idx < effs.size() ) {
      try {
      to_ret = boost::lexical_cast<double>(std::string(effs[idx]));
      } catch (boost::bad_lexical_cast const&) {}
    }
  } catch (cyclus::KeyError &e) { }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Separations::Stream(int element){
  std::string to_ret = waste_stream_();
  try {
    int idx = ElemIdx(element);
    if ( idx < streams.size() ) {
      to_ret = streams[idx];
    }
  } catch (cyclus::KeyError &e) { }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::BeginProcessing(){
  using cyclus::toolkit::ResourceBuff;

  while (!rawbuff_.empty()){
    try {
      processing[context()->time()].Push(rawbuff_.Pop(ResourceBuff::BACK));
      LOG(cyclus::LEV_DEBUG2, "Separartions") << "Separations " << prototype()
                                        << " added resources to processing at t = "
                                        << context()->time();
    } catch(cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparations(cyclus::Context* ctx) {
  return new Separations(ctx);
}

}
