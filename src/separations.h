#ifndef CYCAMORE_SRC_SEPARATIONS_H_
#define CYCAMORE_SRC_SEPARATIONS_H_

#include "cyclus.h"

namespace cycamore {

cyclus::Material::Ptr SepMaterial(std::map<int, double> effs, cyclus::Material::Ptr mat);

class Separations : public cyclus::Facility {
 public:
  Separations(cyclus::Context* ctx);
  virtual ~Separations(){};

  virtual void Tick();
  virtual void Tock();
  virtual void EnterNotify();

  virtual void AcceptMatlTrades(const std::vector<std::pair<
      cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses);

  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
  GetMatlRequests();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests);

  virtual void GetMatlTrades(
      const std::vector<cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                            cyclus::Material::Ptr> >& responses);

  #pragma cyclus clone
  #pragma cyclus initfromcopy
  #pragma cyclus infiletodb
  #pragma cyclus initfromdb
  #pragma cyclus schema
  #pragma cyclus annotations
  #pragma cyclus snapshot

  virtual cyclus::Inventories SnapshotInv();
  virtual void InitInv(cyclus::Inventories& inv);

  // the following pragmas are ommitted and generated manually to handle a
  // vector of resource buffers:
  //     #pragma cyclus snapshotinv
  //     #pragma cyclus initinv

 private:
  #pragma cyclus var { \
    "alias": ["streams", "name", ["info", "cap", ["efficiencies", "comp", "eff"]]], \
    "uitype": ["oneormore", "string", ["pair", "double", ["oneormore", "nuclide", "double"]]], \
    "doc": "Output streams for separations.  Each stream must have a unique name," \
           " a max buffer capacity in kg (neg values indicate infinite size), and a set of component efficiencies." \
           " 'comp' is a component to be separated into this stream (e.g. U, Pu, etc.) and 'eff' is the mass fraction of that component that is separated from the feed into this output stream.", \
  }
  std::map<std::string,std::pair<double,std::map<int,double> > > streams_;

  #pragma cyclus var { \
    "doc" : "Maximum quantity of feed material that can be processed per time step.", \
    "units": "kg", \
  }
  double throughput;

  #pragma cyclus var { \
    "doc" : "", \
  }
  double feedbuf_size;

  #pragma cyclus var { \
    "capacity" : "feedbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> feed;

  #pragma cyclus var { \
    "doc" : "", \
  }
  double leftoverbuf_size;

  #pragma cyclus var { \
    "capacity" : "leftoverbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> leftover;

  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> > streambufs;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SEPARATIONS_H_
