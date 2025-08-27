#ifndef CYCAMORE_SRC_TARIFF_REGION_H_
#define CYCAMORE_SRC_TARIFF_REGION_H_

#include "cyclus.h"
#include <string>
#include <algorithm>
#include <limits>

namespace cycamore {

class TariffRegion : public cyclus::Region {
 public:
  TariffRegion(cyclus::Context* ctx);
  virtual ~TariffRegion();

  virtual void EnterNotify();

  // Required DRE Functions
  virtual void AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs);
  virtual void AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs);

  #pragma cyclus

 private:
  // Template function to reduce code duplication between AdjustMatlPrefs and 
  // AdjustProductPrefs
  template<typename T>
  void AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs) {
    for (auto& req_pair : prefs) {
      for (auto& bid_pair : req_pair.second) {
        cyclus::Bid<T>* bid = bid_pair.first;

        // The supplier should always be a facility, so we can cast to that
        cyclus::Facility* supplier = dynamic_cast<cyclus::Facility*>(bid->bidder()->manager());
        cyclus::Region* supplier_region = supplier->GetRegion();
        
        // If the supplier is in the region list, apply the appropriate adjustment
        auto it = std::find(region_names.begin(), region_names.end(), 
            supplier_region->prototype());
        if (it != region_names.end()) {
          double cost_multiplier = 1.0 + adjustments[it - region_names.begin()];
          double pref_multiplier = 1.0 / cost_multiplier;
          double inf = std::numeric_limits<double>::infinity(); 

          bid_pair.second *= cost_multiplier > 0.0 ? pref_multiplier : inf; 
        }
      }
    }
  }
  // clang-format off
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
  // clang-format on
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
