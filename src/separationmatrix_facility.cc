#include "separationmatrix_facility.h"

namespace separationmatrix {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationmatrixFacility::SeparationmatrixFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationmatrixFacility::str() {
  return Facility::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Tick() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Tock() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationmatrixFacility::inventory_quantity(std::string commod)const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = inventory.find(commod);
  double amt;
  if ( found != inventory.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationmatrixFacility::inventory_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for( it = inventory.begin(); it != inventory.end(); ++it) {
    total += inventory_quantity((*it).first);
  }
  return total;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparationmatrixFacility(cyclus::Context* ctx) {
  return new SeparationmatrixFacility(ctx);
}

}
