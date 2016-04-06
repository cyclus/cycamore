
#ifndef CYCAMORE_SRC_MIXING_FAB_H_
#define CYCAMORE_SRC_MIXING_FAB_H_

#include <string>
#include "cyclus.h"
#include "cycamore_version.h"

namespace cycamore {
  
  
  
  
class MixingFab : public cyclus::Facility {
  public:
    MixingFab(cyclus::Context* ctx) {};
    virtual MixingFab() {};
    
  #pragma cyclus
  
  virtual void Tick(){};
  virtual void Tock(){};
  virtual void EnterNotify();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
      cyclus::CommodMap<cyclus::Material>::type& commods_requests);
  
  virtual void GetMatlTrades(
      const std::vector<cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);
  
  virtual void AcceptMatlTrades(const std::vector<std::pair<
      cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses);
  
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  
  private:
    
    
  #pragma cyclus var { \
    "doc": "Ordered list of commodities on which to requesting stream material.", \
    "uilabel": "Stream Commodities", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> commods_name;
  
  #pragma cyclus var { \
  "doc": "Size of material stream inventory.", \
  "uilabel": " Stream Inventory Capacity", \
  "units": "kg", \
  }
  std:vector<double> commods_size;
  
  
  #pragma cyclus var {"capacity": "commods_size"}
  std::vector< cyclus::toolkit::ResBuf<cyclus::Material> > commods_inv;

  
  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Comodity Fraction", \
    "doc": "Mixing frqction of each commodity requested (same order)." \
    " If unspecified, default is to use 1/N for each fraction.", \
  }
  std::vector<double> commods_frac;
  
  
  #pragma cyclus var { \
    "doc": "Commodity on which to offer/supply mixed fuel material.", \
    "uilabel": "Output Commodity", \
    "uitype": "outcommodity", \
  }
  std::string outcommod;
  
  #pragma cyclus var { \
    "doc": "Maximum number of kg of fuel material that can be supplied per time step.", \
    "uilabel": "Maximum Throughput", \
    "units": "kg", \
  }
  double throughput;
  
  
  // intra-time-step state - no need to be a state var
  // map<request, inventory name>
  std::map<cyclus::Request<cyclus::Material>*, std::string> req_inventories_;
 };
  
  
} // namespace cycamore


#endif  // CYCAMORE_SRC_MIXING_FAB_H_
