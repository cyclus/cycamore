
#ifndef CYCAMORE_SRC_MIXING_FAB_H_
#define CYCAMORE_SRC_MIXING_FAB_H_

#include <string>
#include "cyclus.h"
#include "cycamore_version.h"

namespace cycamore {
  
  /// MixingFab takes in N streams of material and mixes them accordingly
  /// to the ratios prodided by the user.
  ///
  /// The MixingFab has N input inventories: one for each streams to be mixed,
  /// and one output stream.
  ///
  /// The supplying of mixed material is constrained by available inventory of
  /// mixed material quantities.
  /// @endcode
  
  
class MixingFab : public cyclus::Facility {
#pragma cyclus note {   	  \
"niche": "mixing facility",				  \
"doc":								\
"Mixing takes in N streams of material and mixes them accordingly" \
"to the ratios prodided by the user." \
"\n\n " \
"The MixingFab has N input inventories: one for each streams to be mixed,"\
"and one output stream."\
"\n\n" \
"The supplying of mixed material is constrained by available inventory of"\
"mixed material quantities.", \
    }
  
  friend class MixingFabTest;

  public:
  MixingFab(cyclus::Context* ctx);
  virtual ~MixingFab() {};
  
  virtual void Tick();
  virtual void Tock(){};
  virtual void EnterNotify();
  
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
    "doc": "Ordered list of commodities on which to requesting stream material, each commodity corresponds to a input stream.", \
    "uilabel": "Stream Commodities", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> in_commods;
  #pragma cyclus var { \
    "default": [], \
    "uilabel": " Stream Preferences", \
    "doc": "Stream commodity request preferences for each of the given commodities (same order)." \
    " If unspecified, default is to use 1.0 for all preferences.", \
  }
  std::vector<double> fill_commod_prefs;
  
  #pragma cyclus var { \
  "doc": "Size of each material stream inventory.", \
  "uilabel": " Stream Inventory Capacity", \
  "units": "kg", \
  }
  std::vector<double> in_buf_size;
  
  
  // custom SnapshotInv and InitInv and EnterNotify are used to persist this
  // state var.
  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> > streambufs;

  
  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Commodities Mass Fraction", \
    "doc": "Mixing mass fraction of each commodity requested (same order)." \
    " If unspecified, default is to use 1/N for each fraction." \
    " Stream ratios can total to any arbitrary value " \
    "and will be automatically normalized internally." \
  }
  std::vector<double> mixing_ratio;
  
  
  #pragma cyclus var { \
    "doc": "Commodity on which to offer/supply mixed fuel material.", \
    "uilabel": "Output Commodity", \
    "uitype": "outcommodity", \
  }
  std::string out_commod;

  
  #pragma cyclus var { \
    "doc" : "Maximum amount of mixed material that can be stored." \
    " If full, the facility halts operation until space becomes " \
    "available.", \
    "uilabel": "Maximum Leftover Inventory", \
    "default": 1e299, \
    "units": "kg", \
  }
  double output_buf_size;
  
  #pragma cyclus var { \
    "capacity" : "output_buf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> output;
  
  
  #pragma cyclus var { \
    "default": 1e299, \
    "doc": "Maximum number of kg of fuel material that can be mixed per time step.", \
    "uilabel": "Maximum Throughput", \
    "units": "kg", \
  }
  double throughput;
  
  
  // intra-time-step state - no need to be a state var
  // map<request, inventory name>
  std::map<cyclus::Request<cyclus::Material>*, std::string> req_inventories_;
  
  //// A policy for sending material
  cyclus::toolkit::MatlSellPolicy sell_policy;


 };
  
  
} // namespace cycamore


#endif  // CYCAMORE_SRC_MIXING_FAB_H_
