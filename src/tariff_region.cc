#include "tariff_region.h"

namespace cycamore {

TariffRegion::TariffRegion(cyclus::Context* ctx)
: cyclus::Region(ctx) {}

TariffRegion::~TariffRegion() {}

void TariffRegion::EnterNotify() {
  Region::EnterNotify();

}

void TariffRegion::AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs) {
  for (auto& req_pair : prefs) {

    // Iterate over the bids in the request portfolio
    for (auto& bid_pair : req_pair.second) {

      // Get the bid
      cyclus::Bid<cyclus::Material>* bid = bid_pair.first;

      // Get the supplier - use manager() to get the Agent* from the Trader*
      cyclus::Agent* supplier = bid->bidder()->manager();

      // Traverse up the hierarchy to get the supplier's region
      cyclus::Region* supplier_region = nullptr;
      cyclus::Agent* current = supplier;
      while (current != nullptr) {
        supplier_region = dynamic_cast<cyclus::Region*>(current);
        if (supplier_region != nullptr) {
          break;  // Found a region
        }
        current = current->parent();
      }
      
      // If the supplier is in the region list, apply the appropriate adjustment
      auto it = std::find(region_names.begin(), region_names.end(), 
          supplier_region->prototype());
      if (it != region_names.end()) {
        double cost_multiplier = 1.0 + adjustments[it - region_names.begin()];
        bid_pair.second *= cost_multiplier > 0.0 ? 1.0 / cost_multiplier : std::numeric_limits<double>::infinity();
      }
    }
  }
}

void TariffRegion::AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs) {

  // Iterate over the preferences
  for (auto& req_pair : prefs) {
    // Iterate over the bids in the request portfolio
    for (auto& bid_pair : req_pair.second) {

      // Get the bid
      cyclus::Bid<cyclus::Product>* bid = bid_pair.first;

      // Get the supplier - use manager() to get the Agent* from the Trader*
      cyclus::Agent* supplier = bid->bidder()->manager();

      // Traverse up the hierarchy to get the supplier's region
      cyclus::Region* supplier_region = nullptr;
      cyclus::Agent* current = supplier;
      while (current != nullptr) {
        supplier_region = dynamic_cast<cyclus::Region*>(current);
        if (supplier_region != nullptr) {
          break;  // Found a region
        }
        current = current->parent();
      }

      // If the supplier is in the region list, apply the appropriate adjustment
      auto it = std::find(region_names.begin(), region_names.end(), 
                      supplier_region->prototype());
      if (it != region_names.end()) {
        double cost_multiplier = 1.0 + adjustments[it - region_names.begin()];
        bid_pair.second *= cost_multiplier > 0.0 ? 1.0 / cost_multiplier : std::numeric_limits<double>::infinity();
      }
    }
  }
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore