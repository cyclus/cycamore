
#ifndef CYCAMORE_SRC_MIXING_FAB_H_
#define CYCAMORE_SRC_MIXING_FAB_H_

#include <string>
#include "cyclus.h"
#include "cycamore_version.h"

namespace cycamore {
  
  
  
  
class MixingFab : public cyclus::Facility {
  public:
  MixingFab(cyclus::Context* ctx);
  virtual ~MixingFab() {};
  
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


  #pragma cyclus clone
  #pragma cyclus initfromcopy
  #pragma cyclus infiletodb
  #pragma cyclus initfromdb
  #pragma cyclus schema
  #pragma cyclus annotations
  #pragma cyclus snapshot
  // the following pragmas are ommitted and the functions are written
  // manually in order to handle the vector of resource buffers:
  //
  //     #pragma cyclus snapshotinv
  //     #pragma cyclus initinv
  
  virtual cyclus::Inventories SnapshotInv();
  virtual void InitInv(cyclus::Inventories& inv);

  
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
  std::vector<double> commods_size;
  
  
  // custom SnapshotInv and InitInv and EnterNotify are used to persist this
  // state var.
  std::vector< cyclus::toolkit::ResBuf<cyclus::Material> > commods_inv;

  
  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Commodities Mass Fraction", \
    "doc": "Mixing mass fraction of each commodity requested (same order)." \
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
    "doc" : "Maximum amount of mixed material that can be stored." \
    " If full, the facility halts operation until space becomes " \
    "available.", \
    "uilabel": "Maximum Leftover Inventory", \
    "default": 1e299, \
    "units": "kg", \
  }
  double outputbuf_size;
  
  #pragma cyclus var { \
    "capacity" : "outputbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> output;
  
  
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
