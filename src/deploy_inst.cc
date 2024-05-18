// Implements the DeployInst class
#include "deploy_inst.h"

namespace cycamore {

DeployInst::DeployInst(cyclus::Context* ctx)
    : cyclus::Institution(ctx),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longitude) {}

DeployInst::~DeployInst() {}

void DeployInst::Build(cyclus::Agent* parent) {
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

void DeployInst::EnterNotify() {
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

void DeployInst::BuildNotify(Agent* a) {
  Register_(a);
}

void DeployInst::DecomNotify(Agent* a) {
  Unregister_(a);
}

void DeployInst::Register_(Agent* a) {
  using cyclus::toolkit::CommodityProducer;

  CommodityProducer* cp_cast = dynamic_cast<CommodityProducer*>(a);
  if (cp_cast != NULL) {
    LOG(cyclus::LEV_INFO3, "mani") << "Registering agent "
                                   << a->prototype() << a->id()
                                   << " as a commodity producer.";
    cyclus::toolkit::CommodityProducerManager::Register(cp_cast);
  }
}

void DeployInst::Unregister_(Agent* a) {
  using cyclus::toolkit::CommodityProducer;

  CommodityProducer* cp_cast = dynamic_cast<CommodityProducer*>(a);
  if (cp_cast != NULL)
    cyclus::toolkit::CommodityProducerManager::Unregister(cp_cast);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DeployInst::WriteProducerInformation(
  cyclus::toolkit::CommodityProducer* producer) {
  using std::set;
  set<cyclus::toolkit::Commodity,
      cyclus::toolkit::CommodityCompare> commodities =
          producer->ProducedCommodities();
  set<cyclus::toolkit::Commodity, cyclus::toolkit::CommodityCompare>::
      iterator it;

  LOG(cyclus::LEV_DEBUG3, "maninst") << " Clone produces " << commodities.size()
                                     << " commodities.";
  for (it = commodities.begin(); it != commodities.end(); it++) {
    LOG(cyclus::LEV_DEBUG3, "maninst") << " Commodity produced: " << it->name();
    LOG(cyclus::LEV_DEBUG3, "maninst") << "           capacity: " <<
                                       producer->Capacity(*it);
    LOG(cyclus::LEV_DEBUG3, "maninst") << "               cost: " <<
                                       producer->Cost(*it);
  }
}

void DeployInst::RecordPosition() {
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

extern "C" cyclus::Agent* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}

}  // namespace cycamore
