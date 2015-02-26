#ifndef CYCAMORE_SRC_FUEL_FAB_H_
#define CYCAMORE_SRC_FUEL_FAB_H_

#include <string>
#include "cyclus.h"

namespace cycamore {

double CosiWeight(cyclus::Composition::Ptr c, const std::string& spectrum);
bool ValidWeights(double w_low, double w_tgt, double w_high);
double LowFrac(double w_low, double w_tgt, double w_high, double eps = 1e-6);
double HighFrac(double w_low, double w_tgt, double w_high, double eps = 1e-6);

class FuelFab : public cyclus::Facility {
 public:
  FuelFab(cyclus::Context* ctx);
  virtual ~FuelFab() {};

  #pragma cyclus

  virtual void Tick() {};
  virtual void Tock() {};
  virtual void EnterNotify();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  virtual void GetMatlTrades(
      const std::vector< cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

 private:
  #pragma cyclus var {}
  std::string fill_commod;
  #pragma cyclus var {}
  std::string fill_recipe;
  #pragma cyclus var { \
    "default": 0, \
  }
  double fill_pref;
  #pragma cyclus var {}
  double fill_size;
  #pragma cyclus var {'capacity': 'fill_size'}
  cyclus::toolkit::ResBuf<cyclus::Material> fill;

  #pragma cyclus var {}
  std::vector<std::string> fiss_commods;
  #pragma cyclus var { \
    "default": [], \
    "doc": "If unspecified, default is to use zero for all preferences.", \
  }
  std::vector<double> fiss_commod_prefs;
  #pragma cyclus var { \
    "default": "", \
    "doc": "If unspecified, default is to use a dummy blank recipe", \
  }
  std::string fiss_recipe;
  #pragma cyclus var {}
  double fiss_size;
  #pragma cyclus var {'capacity': 'fiss_size'}
  cyclus::toolkit::ResBuf<cyclus::Material> fiss;

  #pragma cyclus var { \
    "doc": "", \
    "default": "", \
  }
  std::string topup_commod;
  #pragma cyclus var { \
    "default": "", \
  }
  std::string topup_recipe;
  #pragma cyclus var { \
    "default": 0, \
  }
  double topup_pref;
  #pragma cyclus var { \
    "default": 0, \
  }
  double topup_size;
  #pragma cyclus var {'capacity': 'topup_size'}
  cyclus::toolkit::ResBuf<cyclus::Material> topup;

  #pragma cyclus var {}
  std::string outcommod;

  #pragma cyclus var { \
    "doc": "'fission_spectrum_ave' for fast reactor compositions or 'thermal' for slow reactors.", \
  }
  std::string spectrum;

  #pragma cyclus var {}
  double throughput;
};

} // namespace cycamore


#endif  // CYCAMORE_SRC_FUEL_FAB_H_
