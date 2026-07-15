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
void TariffRegion::AdjustMatlParams(RequestBidMap<Material>::type& rb_map) {
  AdjustParams<cyclus::Material>(rb_map);
}

void TariffRegion::AdjustProductParams(RequestBidMap<Product>::type& rb_map) {
  AdjustParams<cyclus::Product>(rb_map);
}

bool TariffRegion::HasTariffConfiguration() const {
  return !region_commodity_adjustments_.empty() || 
         !region_blanket_adjustments_.empty() || 
         global_blanket_adjustment_.first != 0.0 || 
         !global_commodity_adjustments_.empty();
}

bool TariffRegion::ConfigurationRecorded() const {
  return configuration_recorded_;
}

std::string TariffRegion::GetRegionName(Region* region) const {
  return region->prototype();
}

Adjustment TariffRegion::FindAdjustmentForCommodity(
    Region* region, const std::string& commodity) {
  const std::string region_name = GetRegionName(region);

  // Tiered override system, from most specific to least specific:
  //
  // 1. Region-specific commodity adjustment
  // 2. Region blanket adjustment
  // 3. Global commodity adjustment
  // 4. Global blanket adjustment

  // 1: Look for an adjustment matching both the supplier region and commodity.
  auto region_it = region_commodity_adjustments_.find(region_name);

  if (region_it != region_commodity_adjustments_.end()) {
    const auto& commodity_adjustments = region_it->second;
    auto commodity_it = commodity_adjustments.find(commodity);

    if (commodity_it != commodity_adjustments.end()) {
      return commodity_it->second;
    }
  }

  // 2: Look for a blanket adjustment matching the supplier region.
  // NOTE: A missing entry means that this region inherits the applicable global
  // adjustment. An entry with an adjustment of zero is an explicit exemption
  // from the global adjustment.
  auto region_blanket_it =
      region_blanket_adjustments_.find(region_name);

  if (region_blanket_it != region_blanket_adjustments_.end()) {
    return region_blanket_it->second;
  }

  // 3: Look for a global adjustment matching the commodity.
  auto global_commodity_it =
      global_commodity_adjustments_.find(commodity);

  if (global_commodity_it != global_commodity_adjustments_.end()) {
    return global_commodity_it->second;
  }

  // 4: Use the global blanket adjustment as the final fallback.
  return global_blanket_adjustment_;
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
  if (!HasTariffConfiguration()) {
    configuration_recorded_ = true;
    return;
  }

  for (const auto& region_entry : region_commodity_adjustments_) {
    const std::string& region_name = region_entry.first;
    const auto& commodity_adjustments = region_entry.second;

    for (const auto& commodity_entry : commodity_adjustments) {
      const std::string& commodity = commodity_entry.first;
      const Adjustment& adjustment = commodity_entry.second;

      context()->NewDatum("CommoditySpecificTariffs")
          ->AddVal("AgentId", id())
          ->AddVal("Time", context()->time())
          ->AddVal("Region", region_name)
          ->AddVal("Commodity", commodity)
          ->AddVal("Adjustment", adjustment.first)
          ->AddVal("Type", adjustment.second)
          ->Record();
    }
  }

  for (const auto& region_entry : region_blanket_adjustments_) {
    const std::string& region_name = region_entry.first;
    const Adjustment& blanket = region_entry.second;

    context()->NewDatum("RegionBlanketTariffs")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Region", region_name)
        ->AddVal("Adjustment", blanket.first)
        ->AddVal("Type", blanket.second)
        ->Record();
  }

  if (global_blanket_adjustment_.first != 0.0) {
    context()->NewDatum("GlobalBlanketTariff")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Adjustment", global_blanket_adjustment_.first)
        ->AddVal("Type", global_blanket_adjustment_.second)
        ->Record();
  }

  for (const auto& global_entry : global_commodity_adjustments_) {
    const std::string& commodity = global_entry.first;
    const Adjustment& adjustment = global_entry.second;

    context()->NewDatum("GlobalCommodityTariffs")
        ->AddVal("AgentId", id())
        ->AddVal("Time", context()->time())
        ->AddVal("Commodity", commodity)
        ->AddVal("Adjustment", adjustment.first)
        ->AddVal("Type", adjustment.second)
        ->Record();
  }

  configuration_recorded_ = true;
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore