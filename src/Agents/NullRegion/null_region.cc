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
    : cyclus::Region(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::~NullRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructNullRegion(cyclus::Context* ctx) {
  return new NullRegion(ctx);
}

/* -------------------- */
} // namespace cycamore
