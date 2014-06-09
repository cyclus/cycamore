#include "commodconverter_facility.h"

namespace commodconverter {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterFacility::CommodconverterFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema commodconverter::CommodconverterFacility

#pragma cyclus def annotations commodconverter::CommodconverterFacility

#pragma cyclus def initinv commodconverter::CommodconverterFacility

#pragma cyclus def snapshotinv commodconverter::CommodconverterFacility

#pragma cyclus def initfromdb commodconverter::CommodconverterFacility

#pragma cyclus def initfromcopy commodconverter::CommodconverterFacility

#pragma cyclus def infiletodb commodconverter::CommodconverterFacility

#pragma cyclus def snapshot commodconverter::CommodconverterFacility

#pragma cyclus def clone commodconverter::CommodconverterFacility

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string CommodconverterFacility::str() {
  return Facility::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::Tick() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CommodconverterFacility::Tock() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructCommodconverterFacility(cyclus::Context* ctx) {
  return new CommodconverterFacility(ctx);
}

} // namespace commodconverter
