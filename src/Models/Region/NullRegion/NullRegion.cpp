// NullRegion.cpp
// Implements the NullRegion class

#include "NullRegion.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::NullRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullRegion::~NullRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructNullRegion() {
      return new NullRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructNullRegion(Model* model) {
      delete model;
}

/* -------------------- */
