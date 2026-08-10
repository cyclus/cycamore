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
  if (!configuration_recorded_) {
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

Adjustment TariffRegion::FindAdjustmentForCommodity(
    Region* region, const std::string& commodity) {
  const std::string region_name = region->prototype();
  const std::string wildcard = "*";
  const std::pair<std::string, std::string> candidates[] = {
      {region_name, commodity},
      {region_name, wildcard},
      {wildcard, commodity},
      {wildcard, wildcard},
  };

  for (const auto& candidate : candidates) {
    auto region_it = adjustments_.find(candidate.first);
    if (region_it == adjustments_.end()) {
      continue;
    }
    auto commodity_it = region_it->second.find(candidate.second);
    if (commodity_it != region_it->second.end()) {
      return commodity_it->second;
    }
  }

  return Adjustment(0.0, "unit_cost");
}

void TariffRegion::ValidateConfiguration() {

  for (const auto& region_entry : adjustments_) {
    if (region_entry.second.empty()) {
      std::string msg = "Adjustment region '" + region_entry.first +
                        "' must contain at least one commodity rule.";
      throw cyclus::ValueError(cyclus::Agent::InformErrorMsg(msg));
    }
    for (const auto& commodity_entry : region_entry.second) {
      const std::string& type = commodity_entry.second.second;
      if (type != "unit_cost" && type != "arc_cost") {
        std::string msg = "Adjustment for region '" + region_entry.first +
                          "' and commodity '" + commodity_entry.first +
                          "' must have type unit_cost or arc_cost. Was: " + type;
        throw cyclus::ValueError(cyclus::Agent::InformErrorMsg(msg));
      }
    }
  }
}

void TariffRegion::RecordTariffConfiguration() {

  for (const auto& region_entry : adjustments_) {
    const std::string& region_name = region_entry.first;
    const auto& commodity_adjustments = region_entry.second;

    for (const auto& commodity_entry : commodity_adjustments) {
      const std::string& commodity = commodity_entry.first;
      const Adjustment& adjustment = commodity_entry.second;

      // Add a flat table summarizing adjustments in the simulation for human
      // readability. Without this, it might be difficult to understand what the
      // region is configured to do from just the database.
      context()->NewDatum("TariffAdjustments")
          ->AddVal("AgentId", id())
          ->AddVal("Time", context()->time())
          ->AddVal("Region", region_name)
          ->AddVal("Commodity", commodity)
          ->AddVal("Adjustment", adjustment.first)
          ->AddVal("Type", adjustment.second)
          ->Record();
    }
  }

  configuration_recorded_ = true;
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore
