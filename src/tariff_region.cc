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


void TariffRegion::Tock() {
  // Record tariff configuration to database only once during first tock
  if (!configuration_recorded_) {
    CLOG(cyclus::LEV_INFO3) << "TariffRegion: Recording configuration to database during Tock";
    RecordTariffConfiguration();
    configuration_recorded_ = true;
    CLOG(cyclus::LEV_INFO3) << "TariffRegion: Configuration recorded successfully";
  }
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
  
  // Build tariff combinations for database recording
  tariff_combinations_.clear();
  
  for (size_t i = 0; i < region_names.size(); ++i) {
    std::string region_name = region_names[i];
    double flat_adjustment = (i < region_flat_adjustments.size()) ? 
                             region_flat_adjustments[i] : 0.0;
    
    // Add flat adjustment
    tariff_combinations_.emplace_back(region_name, "Flat Adjustment", flat_adjustment);
    
    // Add commodity-specific tariffs
    if (i < region_commodity_counts.size()) {
      size_t start_index = 0;
      for (size_t j = 0; j < i; ++j) {
        if (j < region_commodity_counts.size()) {
          start_index += region_commodity_counts[j];
        }
      }
      
      for (size_t j = 0; j < region_commodity_counts[i]; ++j) {
        size_t commodity_index = start_index + j;
        if (commodity_index < region_commodities.size() && 
            commodity_index < region_adjustments.size()) {
          
          tariff_combinations_.emplace_back(
            region_name, 
            region_commodities[commodity_index], 
            region_adjustments[commodity_index]
          );
        }
      }
    }
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

// A simple validation check (no database recording)
void TariffRegion::ValidateConfiguration() {
  // Simple validation: check if all input vectors have the same length
  if (!region_names.empty() && 
      (region_names.size() != region_commodity_counts.size() ||
       region_names.size() != region_flat_adjustments.size())) {
    CLOG(cyclus::LEV_WARN) << "TariffRegion: Input vector length mismatch. "
                   << "All input vectors should have the same length. "
                   << "Using flat adjustments for missing regions.";
  }
}

void TariffRegion::RecordTariffConfiguration() {
  // Safety check: only record if we have valid data
  if (tariff_combinations_.empty()) {
    return;  // No configuration to record
  }
  
  // Record each tariff combination
  for (const auto& combo : tariff_combinations_) {
    std::string region_name = std::get<0>(combo);
    std::string commodity = std::get<1>(combo);
    double adjustment = std::get<2>(combo);
    
    context()->NewDatum("TariffSummaryTable")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Region", region_name)
        ->AddVal("Commodity", commodity)
        ->AddVal("Adjustment", adjustment)  // Convert to percentage
        ->Record();
  }
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore