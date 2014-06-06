#include "commodconverter_inst.h"

using commodconverter::CommodconverterInst;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterInst::CommodconverterInst(cyclus::Context* ctx)
    : cyclus::Institution(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CommodconverterInst::~CommodconverterInst() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string CommodconverterInst::str() {
  return Institution::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructCommodconverterInst(cyclus::Context* ctx) {
  return new CommodconverterInst(ctx);
}
