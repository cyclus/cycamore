#include "tariff_region.h"

namespace cycamore {

TariffRegion::TariffRegion(cyclus::Context* ctx)
: cyclus::Region(ctx) {}

TariffRegion::~TariffRegion() {}

void TariffRegion::EnterNotify() {
  Region::EnterNotify();
  BuildRegionSet();
  ValidateConfiguration();
  BuildTariffRules();
}

void TariffRegion::Tock() {
    RecordTariffConfiguration();
}

// Actual implementation of the DRE Functions using the template function:
void TariffRegion::AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs) {
  AdjustPrefsImpl<cyclus::Material>(prefs);
}

void TariffRegion::AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs) {
  AdjustPrefsImpl<cyclus::Product>(prefs);
}

void TariffRegion::BuildRegionSet() {
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
        adjustment_regions_.insert(region);
      }
    }
  }
}

cyclus::Region* TariffRegion::FindMatchingRegion(cyclus::Facility* supplier) {
  // Get all parent regions of the supplier
  std::vector<cyclus::Region*> parent_regions = supplier->GetAllParentRegions();
  
  // Check each parent region to see if it's in our tariff list
  for (cyclus::Region* parent_region : parent_regions) {
    if (adjustment_regions_.find(parent_region) != adjustment_regions_.end()) {
      return parent_region;
    }
  }
  
  // No matching region found
  return nullptr;
}

void TariffRegion::BuildTariffRules() {
  tariff_rules_.clear();
  region_flat_adjustments_map_.clear();
  
  // Build tariff rules from the input configuration
  for (size_t i = 0; i < region_names.size(); ++i) {
    std::string region_name = region_names[i];
    double flat_adjustment = (i < region_flat_adjustments.size()) ? 
                             region_flat_adjustments[i] : 0.0;
    
    // Store flat adjustment for this region
    region_flat_adjustments_map_[region_name] = flat_adjustment;
    
    // Figure out where in the list of commodities we start for this region
    // Necessary because of the way we've implemented the input file
    if (i < region_commodity_counts.size()) {
      size_t start_index = 0;
      for (size_t j = 0; j < i; ++j) {
        if (j < region_commodity_counts.size()) {
          start_index += region_commodity_counts[j];
        }
      }
      
      // Add commodity-specific tariff rules
      for (size_t j = 0; j < region_commodity_counts[i]; ++j) {
        size_t commodity_index = start_index + j;
        if (commodity_index < region_commodities.size() && 
            commodity_index < region_adjustments.size()) {
          
          tariff_rules_.emplace_back(
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
  
  // First, look for a specific commodity tariff
  for (const auto& rule : tariff_rules_) {
    if (rule.region_name == region_name && rule.commodity == commodity) {
      return rule.adjustment;
    }
  }
  
  // If no specific tariff found, look for flat adjustment for this region
  size_t region_index = 0;
  for (size_t i = 0; i < region_names.size(); ++i) {
    if (region_names[i] == region_name) {
      region_index = i;
      break;
    }
  }
  
  // Return flat adjustment if it exists for this region
  if (region_index < region_flat_adjustments.size()) {
    return region_flat_adjustments[region_index];
  }
  
  // No tariff found, return 0.0
  return 0.0;
}

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
  if (region_names.empty()) {
    return;  // No configuration to record
  }

  // Record each tariff rule (specific commodity tariffs)
  for (const auto& rule : tariff_rules_) {
    context()->NewDatum("CommoditySpecificTariffs")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Region", rule.region_name)
        ->AddVal("Commodity", rule.commodity)
        ->AddVal("Adjustment", rule.adjustment)
        ->Record();
  }
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore