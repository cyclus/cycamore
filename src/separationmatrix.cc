#include "separationmatrix.h"

namespace separationmatrix {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationMatrix::SeparationMatrix(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the SeparationMatrix is experimental.");
    };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema separationmatrix::SeparationMatrix

#pragma cyclus def annotations separationmatrix::SeparationMatrix

#pragma cyclus def initinv separationmatrix::SeparationMatrix

#pragma cyclus def snapshotinv separationmatrix::SeparationMatrix

#pragma cyclus def infiletodb separationmatrix::SeparationMatrix

#pragma cyclus def snapshot separationmatrix::SeparationMatrix

#pragma cyclus def clone separationmatrix::SeparationMatrix


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::InitFrom(SeparationMatrix* m) {

  #pragma cyclus impl initfromcopy separationmatrix::SeparationMatrix

  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::InitFrom(cyclus::QueryableBackend* b){

  #pragma cyclus impl initfromdb separationmatrix::SeparationMatrix

  std::vector<std::string>::const_iterator it;
  for(it = out_commods.begin(); it != out_commods.end(); ++it ) {
    RegisterProduction(*it, capacity, cost);
  }
  RegisterProduction(waste_stream, capacity, 0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::EnterNotify() {
  Facility::EnterNotify();

  std::vector<std::string>::const_iterator it;
  for(it = out_commods.begin(); it != out_commods.end(); ++it ) {
    RegisterProduction(*it, capacity, cost);
  }
  RegisterProduction(waste_stream, capacity, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationMatrix::str() {
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
     << " commod producer members: " << " produces "
     << prod.str()
     << "'}";
  return ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Tick() {

  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is ticking at time "
                                   << time << " {";

  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tick are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();

  // if lifetime is up, clear self of materials??
  if (current_capacity() > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "SepMtx") << " will request " << current_capacity()
                                       << " kg of " << in_commod << ".";
  }

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tick are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Tock() {
  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is tocking {";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tock are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();

  BeginProcessing_();
  if( ready() >=0 ){
    Separate_();
  }

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tock are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
SeparationMatrix::GetMatlRequests() {
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
SeparationMatrix::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::vector<std::string>::const_iterator it;
  for (it = out_commods.begin(); it != out_commods.end(); ++it) {
    BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                                *it,
                                                &inventory[*it]);
    if (!port->bids().empty()) {
      ports.insert(port);
    }
  }

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr SeparationMatrix::GetBids_(
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  // accept blindly, no judgement, any material that's been matched
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator it;

  // put each in stocks.
  for (it = responses.begin(); it != responses.end(); ++it) {
    stocks.Push(it->second);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationMatrix::inventory_quantity(std::string commod) const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = inventory.find(commod);
  double amt;
  if ( found != inventory.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationMatrix::inventory_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for(it=inventory.begin(); it != inventory.end(); ++it) {
    total += inventory_quantity((*it).first);
  }
  return total;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::RegisterProduction(std::string commod_str, double cap, 
    double cost){
  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(commod_str); 

  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, cap);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(){
  if ( processing.find(ready()) != processing.end() ) {
    Separate_(&processing[ready()]);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(cyclus::toolkit::ResourceBuff* buff){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  while ( !buff->empty() ){
    Material::Ptr back = ResCast<Material>(buff->Pop(ResourceBuff::BACK));
    Separate_(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(cyclus::Material::Ptr mat){
  using cyclus::CompMap;
  using cyclus::Composition;
  using std::make_pair;
  using std::string;

  std::map<string, CompMap> sep_comps;
  CompMap::iterator entry;
  CompMap orig = mat->comp()->mass();
  for (entry = orig.begin(); entry != orig.end(); ++entry){
    int iso = int(entry->first);
    int elem = int(iso/10000000.); // convert iso to element
    double sep = entry->second*Eff_(elem); // access matrix
    string stream = Stream_(elem);
    sep_comps[stream][iso] = sep;
  }
  std::map< string, CompMap >::iterator str;
  for(str=sep_comps.begin(); str!=sep_comps.end(); ++str){
    CompMap to_extract = (*str).second;
    double qty = cyclus::compmath::Sum(to_extract);
    Composition::Ptr c = Composition::CreateFromMass(to_extract);
    inventory[(*str).first].Push(mat->ExtractComp(qty, c));
  }
  inventory[waste_stream].Push(mat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int SeparationMatrix::ElemIdx_(int element){
  int to_ret = find(elems.begin(), elems.end(), element) - elems.begin();

  if( to_ret > elems.size() ){
    throw cyclus::KeyError("The element was not found in the matrix");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SeparationMatrix::Eff_(int element){
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
std::string SeparationMatrix::Stream_(int element){
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
void SeparationMatrix::BeginProcessing_(){
  using cyclus::toolkit::ResourceBuff;

  LOG(cyclus::LEV_DEBUG2, "SepMtx") << "Sepatations Matrix" << prototype()
                                    << " added resources to processing";
  while (!stocks.empty()){
    try {
      processing[context()->time()].Push(stocks.Pop(ResourceBuff::BACK));
    } catch(cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparationMatrix(cyclus::Context* ctx) {
  return new SeparationMatrix(ctx);
}

}
