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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
std::string GrowthRegion::schema() {
  return
    "<oneOrMore>                                       \n"
    "  <element name = \"commodity\">                  \n"
    "                                                  \n"
    "    <element name = \"name\">                     \n"
    "      <text/>                                     \n"
    "    </element>                                    \n"
    "                                                  \n"
    "    <oneOrMore>                                   \n"
    "      <element name = \"demand\">                 \n"
    "        <element name=\"type\">                   \n"
    "          <text/>                                 \n"
    "        </element>                                \n"
    "        <element name=\"parameters\">             \n"
    "          <text/>                                 \n"
    "        </element>                                \n"
    "        <optional>                                \n"
    "          <element name=\"start_time\">           \n"
    "            <data type=\"nonNegativeInteger\"/>   \n"
    "          </element>                              \n"
    "        </optional>                               \n"
    "      </element>                                  \n"
    "    </oneOrMore>                                  \n"
    "                                                  \n"
    "  </element>                                      \n"
    "</oneOrMore>                                      \n";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::InitFrom(cyclus::InfileTree* qe) {
  cyclus::Region::InitFrom(qe);
  qe = qe->SubTree("agent/*");
  LOG(cyclus::LEV_DEBUG2, "greg") << "A Growth Region is being initialized";

  std::string query = "commodity";
  int nCommodities = qe->NMatches(query);
  // populate supply demand manager info for each commodity
  for (int i = 0; i < nCommodities; i++) {
    cyclus::InfileTree* iqe = qe->SubTree(query, i);

    std::string name = iqe->GetString("name");
    commodities_.insert(cyclus::toolkit::Commodity(name));

    std::string query = "demand";
    int n = iqe->NMatches(query);
    for (int j = 0; j < n; j++) {
      cyclus::InfileTree* jqe = iqe->SubTree(query, j);
      DemandInfo di;
      di.type = jqe->GetString("type");
      di.params = jqe->GetString("parameters");
      di.time = cyclus::OptionalQuery<int>(qe, jqe, "start_time", 0);
      demands_[name].push_back(di);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::InitFrom(GrowthRegion* m) {
  Region::InitFrom(m);
  commodities_ = m->commodities_;
  demands_ = m->demands_;
}
*/

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::AddCommodityDemand(cyclus::toolkit::Commodity commod) {
  std::string name = commod.name();

  // instantiatedemand
  cyclus::toolkit::PiecewiseFunctionFactory pff;
  for (int i = 0; i < demands_[name].size(); i++) {
    DemandInfo di = demands_[name][i];
    cyclus::toolkit::BasicFunctionFactory bff;
    bool continuous = (i != 0); // the first entry is not continuous
    pff.AddFunction(bff.GetFunctionPtr(di.type, di.params), di.time, continuous);
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
