#include "separations.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Separations::Separations(cyclus::Context* ctx) : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Separations facility " \
                                             "is experimental.");
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema cycamore::Separations

#pragma cyclus def annotations cycamore::Separations

#pragma cyclus def initinv cycamore::Separations

#pragma cyclus def snapshotinv cycamore::Separations

#pragma cyclus def infiletodb cycamore::Separations

#pragma cyclus def snapshot cycamore::Separations

#pragma cyclus def clone cycamore::Separations


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::InitFrom(Separations* m) {

  #pragma cyclus impl initfromcopy cycamore::Separations

  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::InitFrom(cyclus::QueryableBackend* b){

  #pragma cyclus impl initfromdb cycamore::Separations

  std::vector<std::string>::const_iterator it;
  for(it = out_commods.begin(); it != out_commods.end(); ++it ) {
    RegisterProduction(*it, capacity, cost);
  }
  RegisterProduction(waste_stream, capacity, 0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::EnterNotify() {
  Facility::EnterNotify();

  std::vector<std::string>::const_iterator it;
  for(it = out_commods.begin(); it != out_commods.end(); ++it ) {
    RegisterProduction(*it, capacity, cost);
  }
  RegisterProduction(waste_stream, capacity, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
     << "     Current Capacity = " << current_capacity() << ",\n"
     << "     Cost = " << cost_() << ",\n"
     << "     Separated Quantity = " << sepbuff_quantity() << ",\n"
     << "     Raw Quantity = " << rawbuff.quantity() << ",\n"
     << " commod producer members: " << " produces "
     << prod.str()
     << "'}";
  return ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Tick() {

  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is ticking at time "
                                   << time << " {";

  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tick are:";
  PrintStatus();
  // if lifetime is up, clear self of materials??
  if (current_capacity() > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "SepMtx") << " will request " << current_capacity()
                                       << " kg of " << in_commod << ".";
  }

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tick are:";
  PrintStatus();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Tock() {
  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is tocking {";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tock are:";
  PrintStatus();

  BeginProcessing_();
  if( ready() >=0 ){
    Separate_();
  }

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tock are:";
  PrintStatus();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
Separations::GetMatlRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;

  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = current_capacity();
  Material::Ptr mat = cyclus::NewBlankMaterial(amt);

  if (amt > cyclus::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    std::vector<std::string>::const_iterator it;
    std::vector<Request<Material>*> mutuals;
    mutuals.push_back(port->AddRequest(mat, this, in_commod));
    port->AddMutualReqs(mutuals);
    ports.insert(port);
  }  // if amt > eps

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
Separations::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::vector<std::string>::const_iterator it;
  for (it = out_commods.begin(); it != out_commods.end(); ++it) {
    BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                                *it,
                                                &sepbuff[*it]);
    if (!port->bids().empty()) {
      ports.insert(port);
    }
  }

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  // for each trade, respond
  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = std::min(it->amt, sepbuff_quantity(commodity));
    // create a material pointer representing what you can offer
    if ( qty > 0 ) {
      Material::Ptr response = TradeResponse_(qty, &sepbuff[commodity]);
      responses.push_back(std::make_pair(*it, response));
    }
    LOG(cyclus::LEV_INFO5, "ComCnv") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << commodity;
  }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  // accept blindly, no judgement, any material that's been matched
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator it;

  // put each in rawbuff 
  for (it = responses.begin(); it != responses.end(); ++it) {
    AddMat_(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr Separations::GetBids_(
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::AddMat_(cyclus::Material::Ptr mat) {
  // Here we do not check that the recipe matches the input recipe.

  LOG(cyclus::LEV_INFO5, "SepMtx") << prototype() << " is initially holding "
                                << rawbuff.quantity() << " total.";

  try {
    rawbuff.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "SepMtx") << prototype() << " added " << mat->quantity()
                                << " of " << in_commod_()
                                << " to its rawbuff, which is holding "
                                << rawbuff.quantity() << " total.";

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::PrintStatus() {
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     ProcessTime: " << process_time_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Input Commodity = " << in_commod_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Process Time = " << process_time_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Maximum Inventory Size = " << max_inv_size_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Capacity = " << capacity_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Current Capacity = " << current_capacity();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Cost = " << cost_();
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Separated Quantity = " << sepbuff_quantity() ;
  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "     Raw Quantity = " << rawbuff.quantity() ;

}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr Separations::TradeResponse_(
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double Separations::sepbuff_quantity(std::string commod) const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = sepbuff.find(commod);
  double amt;
  if ( found != sepbuff.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double Separations::sepbuff_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for(it=sepbuff.begin(); it != sepbuff.end(); ++it) {
    total += sepbuff_quantity((*it).first);
  }
  return total;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::RegisterProduction(std::string commod_str, double cap, 
    double cost){
  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(commod_str); 

  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, cap);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Separate_(){
  if ( processing.find(ready()) != processing.end() ) {
    Separate_(&processing[ready()]);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Separate_(cyclus::toolkit::ResourceBuff* buff){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  while ( !buff->empty() ){
    Material::Ptr back = ResCast<Material>(buff->Pop(ResourceBuff::BACK));
    Separate_(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::Separate_(cyclus::Material::Ptr mat){
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
    double sep = entry->second*tot*Eff_(elem); // access matrix
    string stream = Stream_(elem);
    sep_comps[stream][iso] = sep;
  }
  std::map< string, CompMap >::iterator str;
  for(str=sep_comps.begin(); str!=sep_comps.end(); ++str){
    CompMap to_extract = (*str).second;
    double qty = cyclus::compmath::Sum(to_extract);
    Composition::Ptr c = Composition::CreateFromMass(to_extract);
    sepbuff[(*str).first].Push(mat->ExtractComp(qty, c));
    LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Separations " << prototype()
                                      << " separated quantity : "
                                      << qty << " at t = "
                                      << context()->time();
  }
  sepbuff[waste_stream].Push(mat);
  LOG(cyclus::LEV_DEBUG2, "SepMtx") << "Separations " << prototype()
                                    << " separated material at t = "
                                    << context()->time();
  LOG(cyclus::LEV_DEBUG2, "SepMtx") << "Separations " << prototype()
                                    << " now has a separated quantity : "
                                    << sepbuff_quantity()
                                    << " and a raw quantity : "
                                    << rawbuff.quantity();

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Separations::ElemIdx_(int element){
  int to_ret = find(elems.begin(), elems.end(), element) - elems.begin();

  if( to_ret > elems.size() ){
    throw cyclus::KeyError("The element was not found in the matrix");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Separations::Eff_(int element){
  double to_ret = 0;
  try {
    int idx = ElemIdx_(element);
    if ( idx < effs.size() ) {
      try {
      to_ret = boost::lexical_cast<double>(std::string(effs[idx]));
      } catch (boost::bad_lexical_cast const&) {}
    }
  } catch (cyclus::KeyError &e) { }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Separations::Stream_(int element){
  std::string to_ret = waste_stream_();
  try {
    int idx = ElemIdx_(element);
    if ( idx < streams.size() ) {
      to_ret = streams[idx];
    }
  } catch (cyclus::KeyError &e) { }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Separations::BeginProcessing_(){
  using cyclus::toolkit::ResourceBuff;

  while (!rawbuff.empty()){
    try {
      processing[context()->time()].Push(rawbuff.Pop(ResourceBuff::BACK));
      LOG(cyclus::LEV_DEBUG2, "SepMtx") << "Separations " << prototype()
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
