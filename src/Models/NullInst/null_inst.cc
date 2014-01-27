// null_inst.cc
// Implements the NullInst class

#include "null_inst.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullInst::NullInst(cyclus::Context* ctx)
    : cyclus::InstModel(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullInst::~NullInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructNullInst(cyclus::Context* ctx) {
  return new NullInst(ctx);
}
/* ------------------- */

} // namespace cycamore
