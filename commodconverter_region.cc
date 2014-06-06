#include "commodconverter_region.h"

using commodconverter::CommodconverterRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterRegion::CommodconverterRegion(cyclus::Context* ctx)
    : cyclus::Region(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterRegion::~CommodconverterRegion() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string CommodconverterRegion::str() {
  return Region::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructCommodconverterRegion(cyclus::Context* ctx) {
  return new CommodconverterRegion(ctx);
}
