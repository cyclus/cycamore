#ifndef CYCAMORE_SRC_TARIFF_REGION_H_
#define CYCAMORE_SRC_TARIFF_REGION_H_

#include "cyclus.h"
#include <string>
#include <utility>
#include <map>

namespace cycamore {

using cyclus::RequestBidMap;
using cyclus::Material;
using cyclus::Product;
using cyclus::Region;
using Adjustment = std::pair<double, std::string>;

class TariffRegion : public Region {
  friend class TariffRegionTests;

 public:
  TariffRegion(cyclus::Context* ctx);
  virtual ~TariffRegion();

  virtual void EnterNotify();
  virtual void Tock();

  // Required DRE Functions
  virtual void AdjustMatlParams(RequestBidMap<Material>::type& rb_map);
  virtual void AdjustProductParams(RequestBidMap<Product>::type& rb_map);

 private:

  // Find the appropriate adjustment for a given region and commodity. Exact
  // matches take precedence over the "*" wildcard in this order:
  // region/commodity, region/*, */commodity, */*.
  Adjustment FindAdjustmentForCommodity(Region* region, const std::string& commodity);
  
  // Helper: Check if any tariff configuration exists
  bool HasTariffConfiguration() const;
  
  // Helper: Check if configuration has been recorded
  bool ConfigurationRecorded() const;
  
  // Helper: Get region prototype name (reduces repeated code)
  std::string GetRegionName(Region* region) const;
  
  // Validate the tariff configuration
  void ValidateConfiguration();
  
  // Record tariff configuration to database
  void RecordTariffConfiguration();

  #pragma cyclus

  // Template function to reduce code duplication between Adjust functions
  template<typename T>
  void AdjustParams(typename RequestBidMap<T>::type& rb_map);
  
  // clang-format off
  
  #pragma cyclus var { \
    "default": {}, \
    "alias": [["adjustments", "region"], "name", [["commodities", "item"], "commodity", ["Adjustment", "val", "type"]]], \
    "doc": "Adjustments by supplier region and commodity. The '*' key is a " \
           "wildcard. Rules are selected in this order: exact region and " \
           "commodity, exact region and '*', '*' and exact commodity, then " \
           "'*' and '*'. A present adjustment of zero is an explicit " \
           "exemption and stops wildcard fallback." \
  }
  std::map<std::string, std::map<std::string, std::pair<double, std::string>>> adjustments_;

  // clang-format on
  
  // Flag to track if configuration has been recorded
  bool configuration_recorded_;
  
};

// Template function implementation (must be in header for template instantiation)
template<typename T>
void TariffRegion::AdjustParams(typename RequestBidMap<T>::type& rb_map) {
  for (auto& req_pair : rb_map) {
    cyclus::Request<T>* request = req_pair.first;
    std::string commodity = request->commodity();
    
    for (auto& bid_pair : req_pair.second) {
      cyclus::Bid<T>* bid = bid_pair.first;
      cyclus::Facility* supplier = dynamic_cast<cyclus::Facility*>(bid->bidder()->manager());
      Region* supplier_region = supplier->GetParentRegion();

      if (supplier_region == this) {
        continue;
      }

      Adjustment adjustment = FindAdjustmentForCommodity(supplier_region, commodity);

      if (adjustment.second == "unit_cost") {
        const double original_unit_cost = bid->unit_cost();
        const double adjusted_unit_cost =
            original_unit_cost * (1.0 + adjustment.first);

        bid->unit_cost(adjusted_unit_cost);
        bid_pair.second += adjusted_unit_cost - original_unit_cost; 
      } 
      else if (adjustment.second == "arc_cost") {
        bid_pair.second *= (1.0 + adjustment.first);
      } 
      else {
        std::string msg = "Adjustment configured incorrectly. "
                          "Must be unit_cost or arc_cost. Was: " +
                          adjustment.second;
        throw cyclus::ValueError(cyclus::Agent::InformErrorMsg(msg));
      }
    }
  }
}

} // namespace cycamore

#endif  // CYCAMORE_SRC_TARIFF_REGION_H_
