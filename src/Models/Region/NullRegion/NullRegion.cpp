// NullRegion.cpp
// Implements the NullRegion class

#include "NullRegion.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string NullRegion::str() {
  return RegionModel::str();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructNullRegion() {
      return new NullRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructNullRegion(Model* model) {
      delete model;
}

/* -------------------- */
