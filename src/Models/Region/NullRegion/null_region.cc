// null_region.cc
// Implements the NullRegion class

#include "null_region.h"

namespace cycamore {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::NullRegion(cyclus::Context* ctx) : cyclus::RegionModel(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::~NullRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructNullRegion(cyclus::Context* ctx) {
  return new NullRegion(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructNullRegion(cyclus::Model* model) {
  delete model;
}

/* -------------------- */
} // namespace cycamore
