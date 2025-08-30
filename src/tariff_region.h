#ifndef CYCAMORE_SRC_TARIFF_REGION_H_
#define CYCAMORE_SRC_TARIFF_REGION_H_

#include "cyclus.h"
#include <string>
#include <algorithm>
#include <limits>
#include <set>
#include <utility>
#include <map>
#include <unordered_map>

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
  // Pre-compute the set of region agents for faster lookups
  void BuildRegionSet();
  
  // Find the best matching region and its adjustment index
  std::pair<cyclus::Region*, int> FindMatchingRegion(cyclus::Facility* supplier);
  
  // Find the appropriate tariff for a given region and commodity
  double FindTariffForCommodity(cyclus::Region* region, const std::string& commodity);
  
  // Build fast lookup maps from the nested configuration
  void BuildTariffLookups();
  
  // Validate the tariff configuration
  void ValidateConfiguration();
  
  // Record tariff configuration to database
  void RecordTariffConfiguration();

  #pragma cyclus

 private:
  // Template function to reduce code duplication between AdjustMatlPrefs and 
  // AdjustProductPrefs
  template<typename T>
  void AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs) {
    for (auto& req_pair : prefs) {
      std::string commodity = req_pair.first->commodity();  // The commodity name
      
      for (auto& bid_pair : req_pair.second) {
        cyclus::Bid<T>* bid = bid_pair.first;

        // The supplier should always be a facility, so we can cast to that
        cyclus::Facility* supplier = dynamic_cast<cyclus::Facility*>(bid->bidder()->manager());
        
        // Find if any of the supplier's parent regions match our tariff list
        auto [matching_region, adjustment_index] = FindMatchingRegion(supplier);
        if (matching_region) {
          // Use commodity-specific tariff system
          double adjustment = FindTariffForCommodity(matching_region, commodity);
          
          double cost_multiplier = 1.0 + adjustment;
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
    "doc": "Region names for tariff configuration." \
  }
  std::vector<std::string> region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Commodities for each region (flattened list, use region_commodity_counts to parse)." \
  }
  std::vector<std::string> region_commodities;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Adjustments for each region's commodities (flattened list, use region_commodity_counts to parse)." \
  }
  std::vector<double> region_adjustments;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Number of commodities for each region (used to parse flattened commodity/adjustment lists)." \
  }
  std::vector<int> region_commodity_counts;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Flat adjustment for each region (default tariff for unspecified commodities)." \
  }
  std::vector<double> region_flat_adjustments;

  // Pre-computed set of region agents for faster lookups
  std::set<cyclus::Region*> region_agents_;
  
  // Fast lookup map for performance
  std::map<std::string, size_t> region_lookup_;
  
  // Tariff combinations for database recording
  std::vector<std::tuple<std::string, std::string, double>> tariff_combinations_;
  
  // Flag to track if configuration has been recorded to database
  bool configuration_recorded_ = false;
  // clang-format on
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
