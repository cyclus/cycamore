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
  // Compute aggregated tariff from all importer regions along the hierarchy
  double ComputeAggregatedTariff(cyclus::Facility* supplier, 
                                  cyclus::Facility* requester,
                                  const std::string& commodity);
  
  // Find the Lowest Common Ancestor of two regions
  cyclus::Region* FindLowestCommonAncestor(cyclus::Region* r1, cyclus::Region* r2);
  
  // Get the chain of ancestors from a region up to (and including) root
  std::vector<cyclus::Region*> GetAncestorChain(cyclus::Region* region);
  
  // Find the most specific tariff rule for a supplier's hierarchy from an importer's perspective
  // Returns the tariff value, checking supplier region hierarchy from most to least specific
  double FindMostSpecificTariff(cyclus::Region* importer_region,
                                const std::vector<cyclus::Region*>& supplier_hierarchy,
                                const std::string& commodity);
  
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
    "alias": ["adjustment_regions", "region", ["AdjustmentConfig", "blanket_adjustment", ["commodity_adjustments", "commodity", "adjustment"]]], \
    "doc": "Tariff configuration: map from region name to (blanket_adjustment, commodity_adjustments_map). Each region can have a blanket adjustment for all commodities and specific adjustments per commodity." \
  }
  std::map<std::string, std::pair<double, std::map<std::string, double>>> adjustment_regions_;

  #pragma cyclus var { \
    "default": 0.0, \
    "doc": "Optional global blanket adjustment applied to all regions and commodities." \
  }
  double global_blanket_adjustment;

  #pragma cyclus var { \
    "default": {}, \
    "alias": ["global_commodity_adjustments", "commodity", "adjustment"], \
    "doc": "Optional global commodity adjustments applied to all regions for specific commodities." \
  }
  std::map<std::string, double> global_commodity_adjustments_;

  // clang-format on
  
  // Flag to track if configuration has been recorded
  bool configuration_recorded_;
  
};

// Template function implementation (must be in header for template instantiation)
template<typename T>
void TariffRegion::AdjustPrefsImpl(typename cyclus::PrefMap<T>::type& prefs) {
  for (auto& req_pair : prefs) {
    cyclus::Request<T>* request = req_pair.first;
    std::string commodity = request->commodity();
    cyclus::Facility* requester = dynamic_cast<cyclus::Facility*>(request->requester()->manager());
    
    for (auto& bid_pair : req_pair.second) {
      cyclus::Bid<T>* bid = bid_pair.first;
      cyclus::Facility* supplier = dynamic_cast<cyclus::Facility*>(bid->bidder()->manager());
      
      // Compute aggregated tariff from all importer regions along the hierarchy
      double adjustment = ComputeAggregatedTariff(supplier, requester, commodity);
      
      if (adjustment != 0.0) {
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
