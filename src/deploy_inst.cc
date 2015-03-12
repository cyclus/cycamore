// Implements the DeployInst class
#include "deploy_inst.h"

namespace cycamore {

DeployInst::DeployInst(cyclus::Context* ctx) : cyclus::Institution(ctx) {}

DeployInst::~DeployInst() {}

void DeployInst::Build(cyclus::Agent* parent) {
  cyclus::Institution::Build(parent);
  BuildSched::iterator it;
  for (int i = 0; i < prototypes.size(); i++) {
    std::string proto = prototypes[i];

    std::stringstream ss;
    ss << proto;

    if (lifetimes.size() == prototypes.size()) {
      cyclus::Agent* a = context()->CreateAgent<Agent>(proto);
      a->Agent::lifetime_ = lifetimes[i];

      ss << "_life_" << lifetimes[i];
      proto = ss.str();
      context()->AddPrototype(proto, a);
    }

    int t = build_times[i];
    for (int j = 0; j < n_build[i]; j++) {
      context()->SchedBuild(this, proto, t);
    }
  }
}

extern "C" cyclus::Agent* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}

}  // namespace cycamore
