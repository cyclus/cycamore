#ifndef CYCAMORE_SRC_MIXER_H_
#define CYCAMORE_SRC_MIXER_H_

#include <string>
#include "cycamore_version.h"
#include "cyclus.h"

namespace cycamore {

/// Mixer mixes N streams with fixed, static, user-specified
/// ratios into a single output stream. The Mixer has N input inventories:
/// one for each streams to be mixed, and one output stream. The supplying of
/// mixed material is constrained by available inventory of mixed material
/// quantities.
class Mixer : public cyclus::Facility {
#pragma cyclus note {   	  \
    "niche": "mixing facility",				  \
    "doc": "Mixer mixes N streams with fixed, static, user-specified" \
           " ratios into a single output stream. The Mixer has N input"\
           " inventories: one for each streams to be mixed, and one output"\
           " stream. The supplying of mixed material is constrained by "\
           " available inventory of mixed material quantities.", \
    }

  friend class MixerTest;

 public:
  Mixer(cyclus::Context* ctx);
  virtual ~Mixer(){};

  virtual void Tick();
  virtual void Tock(){};
  virtual void EnterNotify();

  virtual void AcceptMatlTrades(
      const std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                                  cyclus::Material::Ptr> >& responses);

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

 protected:
  #pragma cyclus var { \
    "doc": "Ordered list of commodities on which to request material stream" \
    " material, each commodity corresponds to an input stream.", \
    "uilabel": "Input Commodities", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> in_commods;

  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Input Preferences", \
    "doc": "Stream commodity request preferences for each of the given" \
           "commodities (same order)." \
           " If unspecified, default is 1.0 for all preferences.", \
  }
  std::vector<double> in_commod_prefs;

  #pragma cyclus var { \
    "doc": "Size of input material stream inventory - i.e. the quantity of each" \
           " stream type to keep on-hand)", \
    "uilabel": "Input Inventory Capacity", \
    "units": [ "", "kg"],                  \
  }
  std::vector<double> in_buf_sizes;

  // custom SnapshotInv and InitInv and EnterNotify are used to persist this
  // state var.
  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> > streambufs;

  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Commodities Mass Fraction", \
    "doc": "Mass mixing ratio of each input commodity requested (same order)." \
           " If unspecified, default is to use 1/N for each fraction." \
           " Stream ratios can total to any arbitrary value" \
           " and will be automatically normalized internally." \
  }
  std::vector<double> mixing_ratios;

  #pragma cyclus var {                                                 \
    "doc" : "Commodity on which to offer/supply mixed fuel material.", \
    "uilabel" : "Output Commodity", "uitype" : "outcommodity", }
  std::string out_commod;

  #pragma cyclus var { \
    "doc" : "Maximum amount of mixed material that can be stored." \
            " If full, the facility halts operation until space becomes" \
            " available.", \
    "uilabel": "Maximum Leftover Inventory", \
    "default": 1e299, \
    "units": "kg", \
  }
  double out_buf_size;

  #pragma cyclus var { "capacity" : "out_buf_size", }
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

}  // namespace cycamore

#endif  // CYCAMORE_SRC_MIXER_H_
