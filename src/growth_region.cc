// Implements the GrowthRegion class
#include "growth_region.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::GrowthRegion(cyclus::Context* ctx) : cyclus::Region(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the GrowthRegion is experimental.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::~GrowthRegion() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::AddCommodityDemand(cyclus::toolkit::Commodity commod) {
  // instantiate demand function
  cyclus::toolkit::PiecewiseFunctionFactory pff;
  int ndemands = demand_types.size();
  for (int i = 0; i < ndemands; i++) {
    cyclus::toolkit::BasicFunctionFactory bff;
    bool continuous = (i != 0);  // the first entry is not continuous
    pff.AddFunction(bff.GetFunctionPtr(demand_types[i], demand_params[i]),
                                       demand_times[i], continuous);
  }

  // register the commodity anddemand
  sdmanager_.RegisterCommodity(commod, pff.GetFunctionPtr());
}

void GrowthRegion::EnterNotify() {
  cyclus::Region::EnterNotify();
  std::set<cyclus::Agent*>::iterator it;
  for (it = cyclus::Agent::children().begin();
       it != cyclus::Agent::children().end();
       ++it) {
    Agent* a = *it;
    Register_(a);
  }

  commod_ = cyclus::toolkit::Commodity(commodity_name);
  AddCommodityDemand(commod_);
}

void GrowthRegion::BuildNotify(Agent* a) {
  Register_(a);
}

void GrowthRegion::DecomNotify(Agent* a) {
  Unregister_(a);
}

void GrowthRegion::Register_(cyclus::Agent* agent) {
  using cyclus::toolkit::CommodityProducerManager;
  using cyclus::toolkit::Builder;

  CommodityProducerManager* cpm_cast =
      dynamic_cast<CommodityProducerManager*>(agent);
  if (cpm_cast != NULL) {
    LOG(cyclus::LEV_INFO3, "greg") << "Registering agent "
                                   << agent->prototype() << agent->id()
                                   << " as a commodity producer manager.";
    sdmanager_.RegisterProducerManager(cpm_cast);
  }

  Builder* b_cast = dynamic_cast<Builder*>(agent);
  if (b_cast != NULL) {
    LOG(cyclus::LEV_INFO3, "greg") << "Registering agent "
                                   << agent->prototype() << agent->id()
                                   << " as a builder.";
    buildmanager_.Register(b_cast);
  }
}

void GrowthRegion::Unregister_(cyclus::Agent* agent) {
  using cyclus::toolkit::CommodityProducerManager;
  using cyclus::toolkit::Builder;

  CommodityProducerManager* cpm_cast =
    dynamic_cast<CommodityProducerManager*>(agent);
  if (cpm_cast != NULL)
    sdmanager_.UnregisterProducerManager(cpm_cast);

  Builder* b_cast = dynamic_cast<Builder*>(agent);
  if (b_cast != NULL)
    buildmanager_.Unregister(b_cast);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::Tick() {
  int time = context()->time();
  double demand = sdmanager_.Demand(commod_, time);
  double supply = sdmanager_.Supply(commod_);
  double unmetdemand = demand - supply;

  LOG(cyclus::LEV_INFO3, "greg") << "GrowthRegion: " << prototype()
                                 << " at time: " << time
                                 << " has the following values regaring "
                                 << " commodity: " << commod_.name();
  LOG(cyclus::LEV_INFO3, "greg") << "  *demand = " << demand;
  LOG(cyclus::LEV_INFO3, "greg") << "  *supply = " << supply;
  LOG(cyclus::LEV_INFO3, "greg") << "  * unmetdemand = " << unmetdemand;

  if (unmetdemand > 0) {
    OrderBuilds(commod_, unmetdemand);
  }
  cyclus::Region::Tick();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::OrderBuilds(cyclus::toolkit::Commodity& commodity,
                               double unmetdemand) {
  using std::vector;
  vector<cyclus::toolkit::BuildOrder> orders =
    buildmanager_.MakeBuildDecision(commodity, unmetdemand);

  LOG(cyclus::LEV_INFO3, "greg") << "The build orders have been determined. "
                                 << orders.size()
                                 << " different type(s) of prototypes will be built.";

  for (int i = 0; i < orders.size(); i++) {
    cyclus::toolkit::BuildOrder order = orders.at(i);
    cyclus::Institution* instcast = dynamic_cast<cyclus::Institution*>(order.builder);
    cyclus::Agent* agentcast = dynamic_cast<cyclus::Agent*>(order.producer);
    if (!instcast || !agentcast) {
      throw cyclus::CastError("growth_region.has tried to incorrectly cast an already known entity.");
    }

    LOG(cyclus::LEV_INFO3, "greg") << "A build order for " << order.number
                                   << " prototype(s) of type "
                                   << dynamic_cast<cyclus::Agent*>(agentcast)->prototype()
                                   << " from builder " << instcast->prototype()
                                   << " is being placed.";

    for (int j = 0; j < order.number; j++) {
      LOG(cyclus::LEV_DEBUG2, "greg") << "Ordering build number: " << j + 1;
      context()->SchedBuild(instcast, agentcast->prototype());
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructGrowthRegion(cyclus::Context* ctx) {
  return new GrowthRegion(ctx);
}

}  // namespace cycamore
