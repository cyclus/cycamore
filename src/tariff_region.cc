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

std::vector<cyclus::Region*> TariffRegion::GetAncestorChain(cyclus::Region* region) {
  std::vector<cyclus::Region*> chain;
  cyclus::Agent* current = region;
  
  while (current != nullptr) {
    cyclus::Region* current_region = dynamic_cast<cyclus::Region*>(current);
    if (current_region != nullptr) {
      chain.push_back(current_region);
    }
    current = current->parent();
  }
  
  return chain;
}

cyclus::Region* TariffRegion::FindLowestCommonAncestor(cyclus::Region* r1, cyclus::Region* r2) {
  // Get ancestor chains for both regions
  std::vector<cyclus::Region*> chain1 = GetAncestorChain(r1);
  std::vector<cyclus::Region*> chain2 = GetAncestorChain(r2);
  
  for (cyclus::Region* ancestor1 : chain1) {
    for (cyclus::Region* ancestor2 : chain2) {
      if (ancestor1 == ancestor2) {
        return ancestor1;  // Found the lowest common ancestor
      }
    }
  }
  
  // No common ancestor found (shouldn't happen in a well-formed simulation)
  return nullptr;
}

double TariffRegion::FindMostSpecificTariff(cyclus::Region* importer_region,
                                             const std::vector<cyclus::Region*>& supplier_hierarchy,
                                             const std::string& commodity) {
  // Check if this importer region is a TariffRegion with configuration
  TariffRegion* tariff_importer = dynamic_cast<TariffRegion*>(importer_region);
  if (tariff_importer == nullptr || !tariff_importer->HasTariffConfiguration()) {
    return 0.0;  // Not a TariffRegion or has no configuration
  }
  
  // Check supplier hierarchy from most specific (supplier itself) to least specific (root)
  for (cyclus::Region* supplier_ancestor : supplier_hierarchy) {
    std::string supplier_ancestor_name = GetRegionName(supplier_ancestor);
    
    // Check if this tariff region has any configuration for this supplier ancestor
    auto region_it = tariff_importer->adjustment_regions_.find(supplier_ancestor_name);
    if (region_it != tariff_importer->adjustment_regions_.end()) {
      // Found a match! Now apply the tiered override system for this specific region
      return tariff_importer->FindTariffForCommodity(supplier_ancestor, commodity);
    }
  }
  
  // No specific rule found for any level of supplier hierarchy
  // Check if there are global rules (commodity-specific or blanket)
  auto global_commodity_it = tariff_importer->global_commodity_adjustments_.find(commodity);
  if (global_commodity_it != tariff_importer->global_commodity_adjustments_.end()) {
    return global_commodity_it->second;
  }
  
  return tariff_importer->global_blanket_adjustment;
}

double TariffRegion::ComputeAggregatedTariff(cyclus::Facility* supplier,
                                              cyclus::Facility* requester,
                                              const std::string& commodity) {
  // Get the parent regions for both facilities
  std::vector<cyclus::Region*> supplier_regions = supplier->GetAllParentRegions();
  std::vector<cyclus::Region*> requester_regions = requester->GetAllParentRegions();
  
  // Handle edge cases
  if (supplier_regions.empty() || requester_regions.empty()) {
    return 0.0;  // No regions to compare
  }
  
  // Get the immediate parent regions (first in the list)
  cyclus::Region* supplier_region = supplier_regions[0];
  cyclus::Region* requester_region = requester_regions[0];
  
  // If they're in the same region, no tariff applies
  if (supplier_region == requester_region) {
    return 0.0;
  }
  
  // Find the Lowest Common Ancestor
  cyclus::Region* lca = FindLowestCommonAncestor(supplier_region, requester_region);
  
  // Get supplier's full ancestor chain (for "most specific" lookups)
  std::vector<cyclus::Region*> supplier_hierarchy = GetAncestorChain(supplier_region);
  
  // Get importer chain: from requester up to (but not including) LCA
  std::vector<cyclus::Region*> importer_chain;
  cyclus::Agent* current = requester_region;
  while (current != nullptr && current != lca) {
    cyclus::Region* current_region = dynamic_cast<cyclus::Region*>(current);
    if (current_region != nullptr) {
      importer_chain.push_back(current_region);
    }
    current = current->parent();
  }
  
  // Aggregate tariffs from all importer regions
  double total_tariff = 0.0;
  for (cyclus::Region* importer : importer_chain) {
    double tariff = FindMostSpecificTariff(importer, supplier_hierarchy, commodity);
    total_tariff += tariff;
  }
  
  return total_tariff;
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