#include "tariff_region.h"

namespace cycamore {

TariffRegion::TariffRegion(cyclus::Context* ctx)
: cyclus::Region(ctx) {}

TariffRegion::~TariffRegion() {}

void TariffRegion::EnterNotify() {
  Region::EnterNotify();

}

// Actual implementation of the DRE Functions using the template function:
void TariffRegion::AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs) {
  AdjustPrefsImpl<cyclus::Material>(prefs);
}

void TariffRegion::AdjustProductPrefs(cyclus::PrefMap<cyclus::Product>::type& prefs) {
  AdjustPrefsImpl<cyclus::Product>(prefs);
}

extern "C" cyclus::Agent* ConstructTariffRegion(cyclus::Context* ctx) {
  return new TariffRegion(ctx);
}

}  // namespace cycamore