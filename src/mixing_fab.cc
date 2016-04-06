#include "mixing_fab.h"
#include <sstream>

using cyclus::Material;
using cyclus::Composition;
using pyne::simple_xs;

#define SHOW(X)                                                     \
std::cout << std::setprecision(17) << __FILE__ << ":" << __LINE__ \
<< ": " #X " = " << X << "\n"

namespace cycamore {
  
  
  MixingFab::MixingFab(cyclus::Context* ctx)
  : cyclus::Facility(ctx), throughput(0) {
    cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>(
       "the MixingFab archetype "
       "is experimental");
  }
  
  //********************************************//
  cyclus::Inventories MixingFab::SnapshotInv() {
    cyclus::Inventories invs;
    
    // these inventory names are intentionally convoluted so as to not clash
    // with the user-specified stream commods that are used as the MixingFab
    // streams inventory names.
    invs["output-inv-name"] = output.PopNRes(output.count());
    output.Push(invs["output-inv-name"]);
    
    
    
    for( int i = 0; i < commods_name.size(); i++){
      std::string inv_name = "input-" + std::to_string(i) + "inv-name";
      invs[inv_name] = commods_inv[i].PopNRes(commods_inv[i].count());
      commods_inv[i].Push(invs[inv_name]);
    }
    return invs;
  }
  
  //********************************************//
  void MixingFab::InitInv(cyclus::Inventories& inv) {
    inv["output-inv-name"] = output.PopNRes(output.count());
    output.Push(inv["output-inv-name"]);
    
    for( int i = 0; i < commods_name.size(); i++){
      std::string inv_name = "input-" + std::to_string(i) + "inv-name";
      commods_inv[i].Push(inv[inv_name]);
    }
  }
  
  
//********************************************//
  void MixingFab::EnterNotify() {
    cyclus::Facility::EnterNotify();
    
    if (commods_frac.empty()) {
      for (int i = 0; i < commods_name.size(); i++) {
        commods_frac.push_back(1/commods_name.size());
      }
    
    } else if (commods_frac.size() != commods_name.size()) {
      std::stringstream ss;
      ss << "prototype '" << prototype() << "' has " << commods_frac.size()
      << " commodity frqction values, expected " << commods_name.size();
      throw cyclus::ValidationError(ss.str());
    
    } else {
      double frac_sum = 0;
      for( int i = 0; i < commods_frac.size(); i++)
        frac_sum += commods_frac[i];
      
      if(frac_sum != 1.) {
        std::stringstream ss;
        ss << "prototype '" << prototype() << "' has " << commods_frac.size()
        << " commodity frqction values, expected " << commods_name.size();
        cyclus::Warn<cyclus::VALUE_WARNING>(ss.str());
      }
      
      for( int i = 0; i < commods_frac.size(); i++){
        commods_frac[i] *= 1./frac_sum;
      }
      
    }
    
    for( int i = 0; i < commods_name.size(); i++){
      commods_inv[i].capacity(commods_size[i]);
    }

    
  }
  
  void MixingFab::Tick(){
    
    if(output.quantity() < output.capacity()){
      
      double tgt_qty = output.capacity() - output.quantity();
      
      Material::Ptr m;
      for( int i = 0; i < commods_name.size(); i++){
        Composition::Ptr c_ = commods_inv[i].Peek()->comp();
        Material::Ptr m_ = commods_inv[i].Pop(commods_frac[i] *tgt_qty);
        m->Absorb(m_);
      }
      output.Push(m);
      
      // IMPORTANT - output buffer needs to be a single homogenous composition or
      // the inventory mixing constraints for bids don't work
      if (output.count() > 1) {
        output.Push(cyclus::toolkit::Squash(output.PopN(output.count())));
      }
      
    }
    
    
  }
  
//********************************************//
  std::set<cyclus::RequestPortfolio<Material>::Ptr> MixingFab::GetMatlRequests() {
    using cyclus::RequestPortfolio;
    
    std::set<RequestPortfolio<Material>::Ptr> ports;
    
    for( int i = 0; i > commods_name.size(); i++){
      
      if (commods_inv[i].space() > cyclus::eps()) {
        RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
        
        Material::Ptr m = cyclus::NewBlankMaterial(commods_inv[i].space());
        
        cyclus::Request<Material>* r;
        r = port->AddRequest(m, this, commods_name[i], 1., false);
        req_inventories_[r] = std::to_string(i);
        ports.insert(port);
      }
      
    }
    return ports;
  }
  
//********************************************//
  void MixingFab::AcceptMatlTrades(const std::vector<
    std::pair<cyclus::Trade<Material>, Material::Ptr> >& responses) {
    
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >::const_iterator trade;

    for (trade = responses.begin(); trade != responses.end(); ++trade) {
      
      cyclus::Request<Material>* req = trade->first.request;
      Material::Ptr m = trade->second;
      
      int i = std::atoi(req_inventories_[req].c_str());
      
      if( i > commods_name.size()-1){
        throw cyclus::ValueError("cycamore::MixingFab was overmatched on requests");
      }
      else {
        commods_inv[i].Push(m);
      }
    }
    
    req_inventories_.clear();
    
    // IMPORTANT - each buffer needs to be a single homogenous composition or
    // the inventory mixing constraints for bids don't work
    for( int i = 0; i < commods_name[i].size(); i++){
      if (commods_inv[i].count() > 1) {
        commods_inv[i].Push(cyclus::toolkit::Squash(commods_inv[i].PopN(commods_inv[i].count())));
      }
    }
    
  }
  
//********************************************//
  std::set<cyclus::BidPortfolio<Material>::Ptr> MixingFab::GetMatlBids(
    cyclus::CommodMap<Material>::type& commod_requests) {
    using cyclus::Bid;
    using cyclus::BidPortfolio;
    using cyclus::CapacityConstraint;
    using cyclus::Material;
    using cyclus::Request;
    
    
    double max_qty = std::min(throughput, output.quantity());
    LOG(cyclus::LEV_INFO3, "MixingFab") << prototype() << " is bidding up to "
    << max_qty << " kg of " << outcommod;
    LOG(cyclus::LEV_INFO5, "MixingFab") << "stats: " << str();
    
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
      port->AddBid(req, m, this);
    }
    
    CapacityConstraint<Material> cc(max_qty);
    port->AddConstraint(cc);
    ports.insert(port);
    
    return ports;
  }

//********************************************//
  void MixingFab::GetMatlTrades(
                              const std::vector<cyclus::Trade<Material> >& trades,
                              std::vector<std::pair<cyclus::Trade<Material>, Material::Ptr> >&
                              responses) {
    using cyclus::Trade;
    
    std::vector<cyclus::Trade<cyclus::Material> >::const_iterator it;
    double tot = 0;
    for (int i = 0; i < trades.size(); i++) {
      Material::Ptr tgt = trades[i].request->target();
      
      double qty = trades[i].amt;
      Material::Ptr m = output.Pop(qty);
      responses.push_back(std::make_pair(trades[i], m));
    }
  }

extern "C" cyclus::Agent* ConstructMixingFab(cyclus::Context* ctx) {
  return new MixingFab(ctx);
}

}
