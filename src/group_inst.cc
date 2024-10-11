// Implements the GroupInst class
#include "group_inst.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GroupInst::GroupInst(cyclus::Context* ctx) : cyclus::Institution(ctx) { }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GroupInst::~GroupInst() {}


void GroupInst::EnterNotify() {
  cyclus::Institution::EnterNotify();

  for (int i = 0; i < prototypes.size(); i++) {
    std::string s_proto = prototypes[i];
    context()->SchedBuild(this, s_proto);  //builds on next timestep
    BuildNotify(this);
  }
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructGroupInst(cyclus::Context* ctx) {
  return new GroupInst(ctx);
}

}  // namespace cycamore
