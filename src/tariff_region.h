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
#include <vector>

namespace cycamore {

// Simple structure to represent a tariff rule
struct TariffRule {
  std::string region_name;
  std::string commodity;
  double adjustment;
  
  TariffRule(const std::string& region, const std::string& comm, double adj)
      : region_name(region), commodity(comm), adjustment(adj) {}
};

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
  // Build the set of region agents for faster lookups
  void BuildRegionSet();
  
  // Find the best matching region for a supplier
  cyclus::Region* FindMatchingRegion(cyclus::Facility* supplier);
  
  // Find the appropriate tariff for a given region and commodity
  double FindTariffForCommodity(cyclus::Region* region, const std::string& commodity);
  
  // Build tariff rules from the input configuration
  void BuildTariffRules();
  
  // Validate the tariff configuration
  void ValidateConfiguration();
  
  // Record tariff configuration to database
  void RecordTariffConfiguration();

  #pragma cyclus

  // Template function to reduce code duplication between AdjustMatlPrefs and 
  // AdjustProductPrefs
  template<typename T>
  void AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs) {
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
  
  // clang-format off
  #pragma cyclus var { \
    "default": [], \
    "doc": "Region names for tariff configuration." \
  }
  std::vector<std::string> region_names;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Commodities for each region (flattened list, use commodity_counts_per_region to parse)." \
  }
  std::vector<std::string> adjusted_commodities;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Adjustments for each region's commodities (flattened list, use region_commodity_counts to parse)." \
  }
  std::vector<double> commodity_adjustments;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Number of commodities for each region (used to parse flattened commodity/adjustment lists)." \
  }
  std::vector<int> commodity_counts_per_region;

  #pragma cyclus var { \
    "default": [], \
    "doc": "Flat adjustment for each region (default tariff for unspecified commodities)." \
  }
  std::vector<double> region_flat_adjustments;

  // clang-format on
  
  // Pre-computed set of region agents for faster lookups
  std::set<cyclus::Region*> adjustment_regions_;
  
  // Simple tariff rules - much cleaner than the complex nested structures
  std::vector<TariffRule> tariff_rules_;
  
  // map of region names to flat adjustments
  std::map<std::string, double> region_flat_adjustments_map_;
  
};

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
