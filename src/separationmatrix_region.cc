#include "separationmatrix_region.h"

using separationmatrix::SeparationmatrixRegion;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationmatrixRegion::SeparationmatrixRegion(cyclus::Context* ctx)
    : cyclus::Region(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationmatrixRegion::~SeparationmatrixRegion() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationmatrixRegion::str() {
  return Region::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparationmatrixRegion(cyclus::Context* ctx) {
  return new SeparationmatrixRegion(ctx);
}
