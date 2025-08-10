#ifndef CYCAMORE_SRC_TARIFF_REGION_H_
#define CYCAMORE_SRC_TARIFF_REGION_H_

#include "cyclus.h"
#include <string>

namespace cycamore {

class TariffRegion : public cyclus::Region {
 public:
  TariffRegion(cyclus::Context* ctx);
  virtual ~TariffRegion();

  virtual void EnterNotify();
  virtual void AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs);
  virtual void AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs);

  #pragma cyclus

 private:
  cyclus::Region* FindRegionByName(const std::string& name);
  cyclus::Region* FindRegionInHierarchy(cyclus::Agent* agent, const std::string& name);

  #pragma cyclus var { \
    "default": [], \
    "doc": "List of regions that will have a trade adjustment " \
    "(subsidy)" \
  }
  std::vector<std::string> region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Multiplicative tariffs to apply to trades from affected " \
    "regions (percent as decimal, must be in same order as " \
    "region_names)" \
  }
  std::vector<double> tariffs;

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
