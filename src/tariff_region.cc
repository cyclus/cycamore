#include "tariff_region.h"

namespace cycamore {

TariffRegion::TariffRegion(cyclus::Context* ctx)
: cyclus::Region(ctx) {}

TariffRegion::~TariffRegion() {}

void TariffRegion::EnterNotify() {
  Region::EnterNotify();
  BuildRegionSet();
  ValidateConfiguration();
  BuildTariffLookups();
}

// Actual implementation of the DRE Functions using the template function:
void TariffRegion::AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs) {
  AdjustPrefsImpl<cyclus::Material>(prefs);
}

void TariffRegion::AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs) {
  AdjustPrefsImpl<cyclus::Product>(prefs);
}

void TariffRegion::BuildRegionSet() {
  region_agents_.clear();
  
  // Get all agents in the simulation
  const std::set<cyclus::Agent*>& all_agents = context()->GetAgentList();
  
  // Find all regions that match our region_names list
  for (const auto& agent : all_agents) {
    if (agent->kind() == "Region") {
      cyclus::Region* region = static_cast<cyclus::Region*>(agent);
      // Check if this region's prototype matches any in our list
      auto it = std::find(region_names.begin(), region_names.end(), 
                         region->prototype());
      if (it != region_names.end()) {
        region_agents_.insert(region);
      }
    }
  }
}

std::pair<cyclus::Region*, int> TariffRegion::FindMatchingRegion(cyclus::Facility* supplier) {
  // Get all parent regions of the supplier
  std::vector<cyclus::Region*> parent_regions = supplier->GetAllParentRegions();
  
  // Check each parent region to see if it's in our tariff list
  for (cyclus::Region* parent_region : parent_regions) {
    if (region_agents_.find(parent_region) != region_agents_.end()) {
      // Found a match! Find the corresponding adjustment index
      auto it = std::find(region_names.begin(), region_names.end(), 
                         parent_region->prototype());
      if (it != region_names.end()) {
        int index = it - region_names.begin();
        return {parent_region, index};
      }
    }
  }
  
  // No matching region found
  return {nullptr, -1};
}

void TariffRegion::BuildTariffLookups() {
  // Build fast lookup map
  for (size_t i = 0; i < region_names.size(); ++i) {
    // Populate the lookup map with the region name and its index
    region_lookup_[region_names[i]] = i;
  }
}

double TariffRegion::FindTariffForCommodity(cyclus::Region* region, const std::string& commodity) {
  std::string region_name = region->prototype();
  
  // Check if the region is in the list, if not return 0.0
  if (region_lookup_.find(region_name) == region_lookup_.end()) {
    return 0.0;
  }
  
  // We now know the region is in the list, so we can get the index:
  size_t region_index = region_lookup_[region_name];
  
  // Check for commodity-specific tariff first
  if (region_index < region_commodity_counts.size()) {
    // Calculate the starting index for this region's commodities in the flattened list
    size_t start_index = 0;
    for (size_t i = 0; i < region_index; ++i) {
      if (i < region_commodity_counts.size()) {
        start_index += region_commodity_counts[i];
      }
    }
    
    // Search through this region's commodities
    for (size_t j = 0; j < region_commodity_counts[region_index]; ++j) {
      size_t commodity_index = start_index + j;
      if (commodity_index < region_commodities.size() && 
          commodity_index < region_adjustments.size() &&
          region_commodities[commodity_index] == commodity) {
        return region_adjustments[commodity_index];
      }
    }
  }
  
  // Fall back to flat adjustment for this region
  return (region_index < region_flat_adjustments.size()) ? 
         region_flat_adjustments[region_index] : 0.0;
}

// A safety check to validate input since we're using a complex configuration
void TariffRegion::ValidateConfiguration() {
  // Check if we have any commodity-specific tariff configuration
  if (region_names.empty()) {
    return; // No configuration to validate
  }
  
  size_t num_regions = region_names.size();
  
  // Helper lambda function to check list sizes and log warnings
  auto check_list_size = [&](const std::string& list_name, size_t actual_size, 
                             const std::string& fallback_msg) {
    if (actual_size != num_regions) {
      CLOG(cyclus::LEV_WARN) << "TariffRegion: Mismatch in region configuration. "
                     << "Found " << num_regions << " region names but "
                     << actual_size << " " << list_name << ". "
                     << fallback_msg;
    }
  };
  
  // Check high-level list sizes
  check_list_size("commodity counts", region_commodity_counts.size(), 
                  "Using flat adjustments for missing regions.");
  check_list_size("flat adjustments", region_flat_adjustments.size(), 
                  "Using 0.0 tariff for missing flat adjustments.");
  
  // Validate flattened commodity/adjustment lists
  size_t total_expected_commodities = 0;
  for (size_t i = 0; i < region_commodity_counts.size(); ++i) {
    total_expected_commodities += region_commodity_counts[i];
  }
  
  if (region_commodities.size() != total_expected_commodities) {
    CLOG(cyclus::LEV_WARN) << "TariffRegion: Mismatch in commodity list. "
                   << "Expected " << total_expected_commodities 
                   << " commodities based on region_commodity_counts but found "
                   << region_commodities.size() << ". Using flat adjustments.";
  }
  
  if (region_adjustments.size() != total_expected_commodities) {
    CLOG(cyclus::LEV_WARN) << "TariffRegion: Mismatch in adjustment list. "
                   << "Expected " << total_expected_commodities 
                   << " adjustments based on region_commodity_counts but found "
                   << region_adjustments.size() << ". Using flat adjustments.";
  }
  
  // Check individual region configurations
  for (size_t i = 0; i < num_regions; ++i) {
    std::string region_name = region_names[i];
    
    // Check if this region has a flat adjustment
    if (i >= region_flat_adjustments.size()) {
      CLOG(cyclus::LEV_WARN) << "TariffRegion: Region '" << region_name 
                     << "' has no flat adjustment specified. Using 0.0 tariff.";
    }
  }
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore