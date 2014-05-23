// growth_region.cc
// Implements the GrowthRegion class

#include "growth_region.h"


namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::GrowthRegion(cyclus::Context* ctx)
    : cyclus::Region(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the GrowthRegion is experimental.");
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::~GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::AddCommodityDemand(cyclus::toolkit::Commodity commod) {
  std::string name = commod.name();

  // instantiate demand function
  cyclus::toolkit::PiecewiseFunctionFactory pff;
  for (int i = 0; i < ndemands; i++) {
    cyclus::toolkit::BasicFunctionFactory bff;
    bool continuous = (i != 0); // the first entry is not continuous
    pff.AddFunction(bff.GetFunctionPtr(demand_types[i], demand_params[i]), 
                                       demand_times[i], continuous);
  }

  // register the commodity anddemand
  sdmanager_.RegisterCommodity(commod, pff.GetFunctionPtr());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::Build(cyclus::Agent* parent) {
  cyclus::Region::Build(parent);

  std::set<cyclus::toolkit::Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); ++it) {
    AddCommodityDemand(*it);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::BuildNotify(Agent* m) {
    // dyncast
    RegisterCommodityProducerManager(m);
    RegisterBuilder(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::Tick(int time) {
  using std::set;
  set<cyclus::toolkit::Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); it++) {
    cyclus::toolkit::Commodity commodity = *it;
    double demand = sdmanager_.Demand(commodity, time);
    double supply = sdmanager_.Supply(commodity);
    double unmetdemand = demand - supply;

    LOG(cyclus::LEV_INFO3, "greg") << "GrowthRegion: " << prototype()
                                   << " at time: " << time
                                   << " has the following values regaring "
                                   << " commodity: " << commodity.name();
    LOG(cyclus::LEV_INFO3, "greg") << "  *demand = " << demand;
    LOG(cyclus::LEV_INFO3, "greg") << "  *supply = " << supply;
    LOG(cyclus::LEV_INFO3, "greg") << "  * unmetdemand = " << unmetdemand;

    if (unmetdemand > 0) {
      OrderBuilds(commodity, unmetdemand);
    }
  }
  cyclus::Region::Tick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterCommodityProducerManager(cyclus::Agent* child) {
  cyclus::toolkit::CommodityProducerManager* cast =
    dynamic_cast<cyclus::toolkit::CommodityProducerManager*>(child);
  if (!cast) {
    throw cyclus::CastError("Failed to cast to CommodityProducerManager");
  }
  sdmanager_.RegisterProducerManager(cast);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterBuilder(cyclus::Agent* child) {
  cyclus::toolkit::Builder* cast =
    dynamic_cast<cyclus::toolkit::Builder*>(child);
  if (!cast) {
    throw cyclus::CastError("Failed to cast to Builder");
  }
  buildmanager_.Register(cast);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructGrowthRegion(cyclus::Context* ctx) {
  return new GrowthRegion(ctx);
}

} // namespace cycamore
