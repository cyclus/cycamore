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
    "doc": "List of regions that will have an advantageous trade adjustment " \
    "(subsidy)" \
  }
  std::vector<std::string> friend_region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "List of regions that will have a disadvantageous trade adjustment " \
    "(tariff)" \
  }
  std::vector<std::string> enemy_region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Multiplicative tariff penalties to apply to trades from enemy " \
    "regions (percent as decimal, must be in same order as " \
    "enemy_region_names)" \
  }
  std::vector<double> enemy_tariffs;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Multiplicative subsidies to apply to trades from friend regions " \
    "(percent as decimal, must be in same order as friend_region_names)" \
  }
  std::vector<double> friend_subsidies;

  std::vector<cyclus::Region*> friend_regions;
  std::vector<cyclus::Region*> enemy_regions;
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
