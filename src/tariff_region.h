#ifndef CYCAMORE_SRC_TARIFF_REGION_H_
#define CYCAMORE_SRC_TARIFF_REGION_H_

#include "cyclus.h"
#include <string>
#include <limits>
#include <utility>
#include <map>
#include <vector>

namespace cycamore {

class TariffRegion : public cyclus::Region {
 public:
  TariffRegion(cyclus::Context* ctx);
  virtual ~TariffRegion();

  virtual void EnterNotify();
  virtual void Tock();

  // Required DRE Functions
  virtual void AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs);
  virtual void AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs);

 private:
  // Find the best matching region for a supplier
  cyclus::Region* FindMatchingRegion(cyclus::Facility* supplier);
  
  // Find the appropriate tariff for a given region and commodity
  // Uses tiered override system: region-specific commodity > region blanket > 
  // global commodity > global blanket
  double FindTariffForCommodity(cyclus::Region* region, const std::string& commodity);
  
  // Helper: Check if any tariff configuration exists
  bool HasTariffConfiguration() const;
  
  // Helper: Check if configuration has been recorded
  bool ConfigurationRecorded() const;
  
  // Helper: Get region prototype name (reduces repeated code)
  std::string GetRegionName(cyclus::Region* region) const;
  
  // Validate the tariff configuration
  void ValidateConfiguration();
  
  // Record tariff configuration to database
  void RecordTariffConfiguration();

  #pragma cyclus

  // Template function to reduce code duplication between AdjustMatlPrefs and 
  // AdjustProductPrefs
  template<typename T>
  void AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs);
  
  // clang-format off
  #pragma cyclus var { \
    "default": {}, \
    "doc": "Tariff configuration: map from region name to (blanket_adjustment, commodity_adjustments_map). Each region can have a blanket adjustment for all commodities and specific adjustments per commodity." \
  }
  std::map<std::string, std::pair<double, std::map<std::string, double>>> adjustment_regions;

  #pragma cyclus var { \
    "default": 0.0, \
    "doc": "Optional global blanket adjustment applied to all regions and commodities." \
  }
  double global_blanket_adjustment;

  #pragma cyclus var { \
    "default": {}, \
    "doc": "Optional global commodity adjustments applied to all regions for specific commodities." \
  }
  std::map<std::string, double> global_commodity_adjustments;

  // clang-format on
  
  // Flag to track if configuration has been recorded
  bool configuration_recorded_;
  
};

// Template function implementation (must be in header for template instantiation)
template<typename T>
void TariffRegion::AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs) {
  for (auto& req_pair : prefs) {
    std::string commodity = req_pair.first->commodity();
    
    for (auto& bid_pair : req_pair.second) {
      cyclus::Bid<T>* bid = bid_pair.first;
      cyclus::Facility* supplier = dynamic_cast<cyclus::Facility*>(bid->bidder()->manager());
      
      // Find if any of the supplier's parent regions match our tariff list
      cyclus::Region* matching_region = FindMatchingRegion(supplier);
      if (matching_region) {
        double adjustment = FindTariffForCommodity(matching_region, commodity);
        
        double cost_multiplier = 1.0 + adjustment;
        double pref_multiplier = 1.0 / cost_multiplier;
        double inf = std::numeric_limits<double>::infinity(); 

        bid_pair.second *= cost_multiplier > 0.0 ? pref_multiplier : inf; 
      }
    }
  }
}

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
