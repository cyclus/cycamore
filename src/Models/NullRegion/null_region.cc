// null_region.cc
// Implements the NullRegion class

#include "null_region.h"

namespace cycamore {
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::NullRegion(cyclus::Context* ctx)
    : cyclus::RegionModel(ctx),
      cyclus::Model(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::~NullRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructNullRegion(cyclus::Context* ctx) {
  return new NullRegion(ctx);
}

/* -------------------- */
} // namespace cycamore
