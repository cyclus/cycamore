#include "mixing_fab.h"
#include <sstream>

using cyclus::Material;
using cyclus::Composition;
using cyclus::toolkit::ResBuf;

using pyne::simple_xs;

#define SHOW(X)                                                     \
std::cout << std::setprecision(17) << __FILE__ << ":" << __LINE__ \
<< ": " #X " = " << X << "\n"

#define cyDBGL		std::cout << __FILE__ << " : " << __LINE__ << " [" << __FUNCTION__ << "]" << std::endl;
//#define cyDBGL		;

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
    
    std::map<std::string, ResBuf<Material> >::iterator it;
    for (it = streambufs.begin(); it != streambufs.end(); ++it) {
      invs[it->first] = it->second.PopNRes(it->second.count());
      it->second.Push(invs[it->first]);
    }
    return invs;
  }
  
  //********************************************//
  void MixingFab::InitInv(cyclus::Inventories& inv) {
    inv["output-inv-name"] = output.PopNRes(output.count());
    output.Push(inv["output-inv-name"]);
    
    cyclus::Inventories::iterator it;
    for (it = inv.begin(); it != inv.end(); ++it) {
      streambufs[it->first].Push(it->second);
    }
  }
  
  
//********************************************//
  void MixingFab::EnterNotify() {
    cyclus::Facility::EnterNotify();

    if (commods_frac.empty()) {
      for (int i = 0; i < in_commods.size(); i++) {
        commods_frac.push_back(1/in_commods.size());
      }
    
    } else if (commods_frac.size() != in_commods.size()) {
      std::stringstream ss;
      ss << "prototype '" << prototype() << "' has " << commods_frac.size()
      << " commodity frqction values, expected " << in_commods.size();
      throw cyclus::ValidationError(ss.str());
    
    } else {
      double frac_sum = 0;
      for( int i = 0; i < commods_frac.size(); i++)
        frac_sum += commods_frac[i];
      
      if(frac_sum != 1.) {
        std::stringstream ss;
        ss << "prototype '" << prototype() << "' has " << commods_frac.size()
        << " commodity frqction values, expected " << in_commods.size();
        cyclus::Warn<cyclus::VALUE_WARNING>(ss.str());
      }
      
      for( int i = 0; i < commods_frac.size(); i++){
        commods_frac[i] *= 1./frac_sum;
      }
      
    }
    
    for( int i = 0; i < in_commods.size(); i++){
      std::string name = in_commods[i];
      double cap = commods_size[i];
      if (cap >= 0) {
        streambufs[name].capacity(cap);
      }
    }
    
    
    sell_policy.Init(this, &output, "output").Set(outcommod).Start();

    

  }
  
  //********************************************//
  void MixingFab::Tick(){
    
    if(output.quantity() < output.capacity()){

      double tgt_qty = output.space();

      for( int i = 0; i < in_commods.size(); i++){
        std::string name = in_commods[i];
        tgt_qty = std::min(tgt_qty, streambufs[name].quantity()/ commods_frac[i] );
      }
      
      tgt_qty = std::min(tgt_qty, throughput);

      if(tgt_qty > 0){
        Material::Ptr m = cyclus::NewBlankMaterial(tgt_qty);
        for( int i = 0; i < in_commods.size(); i++){

          std::string name = in_commods[i];
          Material::Ptr m_ = streambufs[name].Pop(commods_frac[i] *tgt_qty);

          m->Absorb(m_);
        }

        output.Push(m);
      }
      
    }
    
  }
  
//********************************************//
  std::set<cyclus::RequestPortfolio<Material>::Ptr> MixingFab::GetMatlRequests() {
    using cyclus::RequestPortfolio;

    std::set<RequestPortfolio<Material>::Ptr> ports;
    
    for( int i = 0; i < in_commods.size(); i++){
      std::string name = in_commods[i];

      if (streambufs[name].space() > cyclus::eps()) {
        RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
        
        Material::Ptr m = cyclus::NewBlankMaterial(streambufs[name].space());
        
        cyclus::Request<Material>* r;
        r = port->AddRequest(m, this, in_commods[i], 1., false);
        req_inventories_[r] = name;
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
      
      std::string name = req_inventories_[req];
      bool assigned = false;
      std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> >::iterator it;

      for( it = streambufs.begin(); it != streambufs.end(); it++ ){
        if( name == it->first){
          it->second.Push(m);
          assigned = true;
          break;
        }
      }
      if( !assigned ){
        throw cyclus::ValueError("cycamore::MixingFab was overmatched on requests");
      }
    }
    
    req_inventories_.clear();
    
  }
  
//********************************************//
/*  std::set<cyclus::BidPortfolio<Material>::Ptr> MixingFab::GetMatlBids(
    cyclus::CommodMap<Material>::type& commod_requests) {
    using cyclus::Bid;
    using cyclus::BidPortfolio;
    using cyclus::CapacityConstraint;
    using cyclus::Material;
    using cyclus::Request;
    
    
    double max_qty = output.quantity();
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
    
    std::cout << max_qty << std::endl;
    CapacityConstraint<Material> cc(max_qty);
    port->AddConstraint(cc);
    ports.insert(port);
    
    return ports;
  }
*/
//********************************************//
/*  void MixingFab::GetMatlTrades(
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
  }*/

extern "C" cyclus::Agent* ConstructMixingFab(cyclus::Context* ctx) {
  return new MixingFab(ctx);
}

}
