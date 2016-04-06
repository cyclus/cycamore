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
  : cyclus::Facility(ctx), fill_size(0), fiss_size(0), throughput(0) {
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
    
    for( int k = 0; k > commods_name.size(); k++){
      
      if (commods_inv[i].space() > cyclus::eps()) {
        RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
        
        Material::Ptr m = cyclus::NewBlankMaterial(commods_inv[i].space());
        
        cyclus::Request<Material>* r;
        r = port->AddRequest(m, this, commods_name[i], 1., exclusive);
        req_inventories_[r] = commods_name[i];
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
      std::string commod = trade->first.request->commodity();
      double req_qty = trade->first.request->target()->quantity();
      cyclus::Request<Material>* req = trade->first.request;
      Material::Ptr m = trade->second;
      
      bool match = false;
      
      for( int i = 0; i < commods_name[i].size(); i++){
        
        if (req_inventories_[req] == commods_name[i]) {
          commods_inv[i].Push(m);
          match = true; break;
        }
      }
      
      if( !match)
        throw cyclus::ValueError("cycamore::MixingFab was overmatched on requests");

    }
    
    req_inventories_.clear();
    
    // IMPORTANT - each buffer needs to be a single homogenous composition or
    // the inventory mixing constraints for bids don't work
    for( int i = 0; i < commods_name[i].size(); i++){
      if (commods_inv[i].count() > 1) {
        commods_inv[i].Push(cyclus::toolkit::Squash(commods_inv[i].PopN(fill.count())));
      }
    }
    
  }
  
//********************************************//
  std::set<cyclus::BidPortfolio<Material>::Ptr> MixingFab::GetMatlBids(
    cyclus::CommodMap<Material>::type& commods_requests) {
    using cyclus::BidPortfolio;
    
    std::set<BidPortfolio<Material>::Ptr> ports;
    std::vector<cyclus::Request<Material>*>& reqs = commods_requests[outcommod];
    
    
    return ports;
  }

//********************************************//
  void MixingFab::GetMatlTrades(
                              const std::vector<cyclus::Trade<Material> >& trades,
                              std::vector<std::pair<cyclus::Trade<Material>, Material::Ptr> >&
                              responses) {
    using cyclus::Trade;
    
    
    
    
  }
  
  
}
