// Implements the DeployInst class
#include "deploy_manager_inst.h"

namespace cycamore {

DeployManagerInst::DeployManagerInst(cyclus::Context* ctx)
    : cycamore::ManagerInst(ctx),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longitude) {}

DeployManagerInst::~DeployManagerInst() {}

void DeployManagerInst::Build(cyclus::Agent* parent) {
  cyclus::Institution::Build(parent);
  BuildSched::iterator it;
  std::set<std::string> protos;
  for (int i = 0; i < prototypes.size(); i++) {
    std::string proto = prototypes[i];

    std::stringstream ss;
    ss << proto;

    if (lifetimes.size() == prototypes.size()) {
      cyclus::Agent* a = context()->CreateAgent<Agent>(proto);
      if (a->lifetime() != lifetimes[i]) {
        a->lifetime(lifetimes[i]);

        if (lifetimes[i] == -1) {
          ss << "_life_forever";
        } else {
          ss << "_life_" << lifetimes[i];
        }
        proto = ss.str();
        if (protos.count(proto) == 0) {
          protos.insert(proto);
          context()->AddPrototype(proto, a);
        }
      }
    }

    int t = build_times[i];
    for (int j = 0; j < n_build[i]; j++) {
      context()->SchedBuild(this, proto, t);
    }
  }
}

void DeployManagerInst::EnterNotify() {
  cyclus::Institution::EnterNotify();
  int n = prototypes.size();
  if (build_times.size() != n) {
    std::stringstream ss;
    ss << "prototype '" << prototype() << "' has " << build_times.size()
       << " build_times vals, expected " << n;
    throw cyclus::ValueError(ss.str());
  } else if (n_build.size() != n) {
    std::stringstream ss;
    ss << "prototype '" << prototype() << "' has " << n_build.size()
       << " n_build vals, expected " << n;
    throw cyclus::ValueError(ss.str());
  } else if (lifetimes.size() > 0 && lifetimes.size() != n) {
    std::stringstream ss;
    ss << "prototype '" << prototype() << "' has " << lifetimes.size()
       << " lifetimes vals, expected " << n;
    throw cyclus::ValueError(ss.str());
  }
  RecordPosition();
}

void DeployManagerInst::RecordPosition() {
  std::string specification = this->spec();
  context()
      ->NewDatum("AgentPosition")
      ->AddVal("Spec", specification)
      ->AddVal("Prototype", this->prototype())
      ->AddVal("AgentId", id())
      ->AddVal("Latitude", latitude)
      ->AddVal("Longitude", longitude)
      ->Record();
}

extern "C" cyclus::Agent* ConstructDeployManagerInst(cyclus::Context* ctx) {
  return new DeployManagerInst(ctx);
}

}  // namespace cycamore
