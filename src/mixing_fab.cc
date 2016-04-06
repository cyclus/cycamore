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
    cyclus::CommodMap<Material>::type& commods_requests) {
    using cyclus::BidPortfolio;
    
    std::set<BidPortfolio<Material>::Ptr> ports;
    std::vector<cyclus::Request<Material>*>& reqs = commods_requests[outcommod];
    
    
    if (throughput == 0) {
      return ports;
    } else if (reqs.size() == 0) {
      return ports;
    }
    
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
    for (int j = 0; j < reqs.size(); j++) {
      cyclus::Request<Material>* req = reqs[j];
      
      double tgt_qty = req->target()->quantity();
      double max_qty = 1e200;
      Material::Ptr m;
      for( int i = 0; i < commods_name.size(); i++){
        Composition::Ptr c_ = commods_inv[i].Peek()->comp();
        Material::Ptr m_ = Material::CreateUntracked(commods_frac[i] * tgt_qty, c_);
        m->Absorb(m_);
        
        max_qty = std::min(max_qty, commods_inv[i].quantity() /commods_frac[i]);
      }
      
      bool exclusive = false;
      port->AddBid(req, m, this, exclusive);
      cyclus::CapacityConstraint<Material> cc(max_qty);
      port->AddConstraint(cc);
      ports.insert(port);
      
    }
    
    
    
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
      
      tot += qty;
      if (tot > throughput + cyclus::eps()) {
        std::stringstream ss;
        ss << "FuelFab was matched above throughput limit: " << tot << " > "
        << throughput;
        throw cyclus::ValueError(ss.str());
      }
      
      Material::Ptr m;
      for( int i = 0; i < commods_name.size(); i++){
        if(commods_frac[i] * qty > 0){
          m->Absorb( commods_inv[i].Pop( commods_frac[i] * qty) );
        }
      }
      
      responses.push_back(std::make_pair(trades[i], m));
    }
  }

extern "C" cyclus::Agent* ConstructMixingFab(cyclus::Context* ctx) {
  return new MixingFab(ctx);
}

}
