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
  #pragma cyclus var { \
    "default": [], \
    "doc": "List of regions that will have a trade adjustment applied to them." \
  }
  std::vector<std::string> region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Adjustments (multiply by (1+val)) to apply to cost of trades from " \
           "affected regions. Positive values are tariffs, negative values " \
           "are subsidies. Must be in same order as region_names." , \
    "tooltip": "Percent as decimal." \
  }
  std::vector<double> adjustments;

};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
