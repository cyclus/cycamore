#include "tariff_region.h"

namespace cycamore {

TariffRegion::TariffRegion(cyclus::Context* ctx)
: cyclus::Region(ctx), configuration_recorded_(false) {}

TariffRegion::~TariffRegion() {}

void TariffRegion::EnterNotify() {
  Region::EnterNotify();
  ValidateConfiguration();
}

void TariffRegion::Tock() {
  if (!ConfigurationRecorded()) {
    RecordTariffConfiguration();
  }
}

// Actual implementation of the DRE Functions using the template function:
void TariffRegion::AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs) {
  AdjustPrefsImpl<cyclus::Material>(prefs);
}

void TariffRegion::AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs) {
  AdjustPrefsImpl<cyclus::Product>(prefs);
}

bool TariffRegion::HasTariffConfiguration() const {
  return !adjustment_regions_.empty() || global_blanket_adjustment != 0.0 || 
         !global_commodity_adjustments_.empty();
}

bool TariffRegion::ConfigurationRecorded() const {
  return configuration_recorded_;
}

std::string TariffRegion::GetRegionName(cyclus::Region* region) const {
  return region->prototype();
}

cyclus::Region* TariffRegion::FindMatchingRegion(cyclus::Facility* supplier) {
  // Get all parent regions of the supplier
  std::vector<cyclus::Region*> parent_regions = supplier->GetAllParentRegions();
  
  // Check each parent region to see if it's in our tariff configuration
  // We can directly look up by prototype name in the map (O(1) lookup)
  for (cyclus::Region* parent_region : parent_regions) {
    std::string region_name = GetRegionName(parent_region);
    if (adjustment_regions_.find(region_name) != adjustment_regions_.end()) {
      return parent_region;
    }
  }
  
  // No matching region found
  return nullptr;
}

double TariffRegion::FindTariffForCommodity(cyclus::Region* region, const std::string& commodity) {
  std::string region_name = GetRegionName(region);
  
  // Tiered override system (most specific to least specific):
  // 1. Region-specific commodity adjustment (highest priority)
  // 2. Region blanket adjustment
  // 3. Global commodity adjustment
  // 4. Global blanket adjustment (lowest priority)
  
  // Look up the region in our adjustment_regions_ map
  auto region_it = adjustment_regions_.find(region_name);
  if (region_it != adjustment_regions_.end()) {
    const auto& region_data = region_it->second;
    const auto& commodity_adjustments = region_data.second;
    
    // Tier 1: Check for region-specific commodity adjustment
    auto commodity_it = commodity_adjustments.find(commodity);
    if (commodity_it != commodity_adjustments.end()) {
      return commodity_it->second;  // Most specific - override everything
    }
    
    // Tier 2: Use region blanket adjustment
    double region_blanket = region_data.first;
    if (region_blanket != 0.0) {
      return region_blanket;  // Override global adjustments
    }
  }
  
  // Tier 3: Check for global commodity-specific adjustment
  auto global_commodity_it = global_commodity_adjustments_.find(commodity);
  if (global_commodity_it != global_commodity_adjustments_.end()) {
    return global_commodity_it->second;  // Override global blanket
  }
  
  // Tier 4: Use global blanket adjustment (lowest priority)
  return global_blanket_adjustment;
}

void TariffRegion::ValidateConfiguration() {
  // Basic validation: check that adjustment_regions_ map is not empty if we expect tariffs
  // The nested structure itself enforces correctness (no parallel list mismatches possible)
  if (!HasTariffConfiguration()) {
    LOG(cyclus::LEV_INFO1, "TariffRegion") << "No tariff configuration specified. "
                   << "No adjustments will be applied.";
  }
}

void TariffRegion::RecordTariffConfiguration() {
  // Safety check: only record if we have valid data
  if (!HasTariffConfiguration()) {
    configuration_recorded_ = true;  // Mark as recorded even if empty
    return;  // No configuration to record
  }
  
  // Mark as recorded before actually recording (in case recording throws)
  configuration_recorded_ = true;

  // Record region-specific tariff configurations
  for (const auto& region_entry : adjustment_regions_) {
    const std::string& region_name = region_entry.first;
    double blanket_adjustment = region_entry.second.first;
    const auto& commodity_adjustments = region_entry.second.second;
    
    // Record blanket adjustment for this region
    context()->NewDatum("RegionBlanketTariffs")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Region", region_name)
        ->AddVal("Adjustment", blanket_adjustment)
        ->Record();
    
    // Record commodity-specific adjustments for this region
    for (const auto& commodity_entry : commodity_adjustments) {
      context()->NewDatum("CommoditySpecificTariffs")
          ->AddVal("AgentId", id())
          ->AddVal("Time", context()->time())
          ->AddVal("Region", region_name)
          ->AddVal("Commodity", commodity_entry.first)
          ->AddVal("Adjustment", commodity_entry.second)
          ->Record();
    }
  }
  
  // Record global adjustments if present
  if (global_blanket_adjustment != 0.0) {
    context()->NewDatum("GlobalBlanketTariff")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Adjustment", global_blanket_adjustment)
        ->Record();
  }
  
  for (const auto& global_entry : global_commodity_adjustments_) {
    context()->NewDatum("GlobalCommodityTariffs")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Commodity", global_entry.first)
        ->AddVal("Adjustment", global_entry.second)
        ->Record();
  }
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore