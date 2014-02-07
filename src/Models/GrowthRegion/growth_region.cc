// growth_region.cc
// Implements the GrowthRegion class

#include "growth_region.h"

#include "query_engine.h"
#include "symbolic_function_factories.h"
#include "inst_model.h"
#include "error.h"

#include <vector>


namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::GrowthRegion(cyclus::Context* ctx)
    : cyclus::RegionModel(ctx),
      cyclus::Model(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::~GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
void GrowthRegion::InitFrom(cyclus::QueryEngine* qe) {
  cyclus::RegionModel::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());
  LOG(cyclus::LEV_DEBUG2, "greg") << "A Growth Region is being initialized";

  std::string query = "commodity";
  int nCommodities = qe->NElementsMatchingQuery(query);
  // populate supply demand manager info for each commodity
  for (int i = 0; i < nCommodities; i++) {
    cyclus::QueryEngine* iqe = qe->QueryElement(query, i);

    std::string name = iqe->GetElementContent("name");
    commodities_.insert(cyclus::Commodity(name));

    std::string query = "demand";
    int n = iqe->NElementsMatchingQuery(query);
    for (int j = 0; j < n; j++) {
      cyclus::QueryEngine* jqe = iqe->QueryElement(query, j);
      DemandInfo di;
      di.type = jqe->GetElementContent("type");
      di.params = jqe->GetElementContent("parameters");
      di.time = cyclus::GetOptionalQuery<int>(jqe, "start_time", 0);
      demands_[name].push_back(di);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::InitFrom(GrowthRegion* m) {
  RegionModel::InitFrom(m);
  commodities_ = m->commodities_;
  demands_ = m->demands_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::AddCommodityDemand(cyclus::Commodity commod) {
  std::string name = commod.name();

  // instantiatedemand
  cyclus::PiecewiseFunctionFactory pff;
  for (int i = 0; i < demands_[name].size(); i++) {
    DemandInfo di = demands_[name][i];
    cyclus::BasicFunctionFactory bff;
    bool continuous = (i != 0); // the first entry is not continuous
    pff.AddFunction(bff.GetFunctionPtr(di.type, di.params), di.time, continuous);
  }

  // register the commodity anddemand
  sdmanager_.RegisterCommodity(commod, pff.GetFunctionPtr());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::Deploy(cyclus::Model* parent) {
  cyclus::RegionModel::Deploy(parent);
  for (int i = 0; i != children().size(); i++) {
    cyclus::Model* child = children().at(i);
    RegisterCommodityProducerManager(child);
    RegisterBuilder(child);
  }

  std::set<cyclus::Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); ++it) {
    AddCommodityDemand(*it);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::Tick(int time) {
  using std::set;
  set<cyclus::Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); it++) {
    cyclus::Commodity commodity = *it;
    double demand = sdmanager_.Demand(commodity, time);
    double supply = sdmanager_.Supply(commodity);
    double unmetdemand = demand - supply;

    LOG(cyclus::LEV_INFO3, "greg") << "GrowthRegion: " << name()
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
  cyclus::RegionModel::Tick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterCommodity(cyclus::Commodity& commodity) {
  if (commodities_.find(commodity) != commodities_.end()) {
    throw cyclus::KeyError("A GrowthRegion ("
                           + name() + " is trying to register a commodity twice.");
  } else {
    commodities_.insert(commodity);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterCommodityProducerManager(cyclus::Model* child) {
  cyclus::CommodityProducerManager* cast =
    dynamic_cast<cyclus::CommodityProducerManager*>(child);
  if (!cast) {
    throw cyclus::CastError("Failed to cast to CommodityProducerManager");
  }
  sdmanager_.RegisterProducerManager(cast);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterBuilder(cyclus::Model* child) {
  cyclus::Builder* cast =
    dynamic_cast<cyclus::Builder*>(child);
  if (!cast) {
    throw cyclus::CastError("Failed to cast to Builder");
  }
  buildmanager_.RegisterBuilder(cast);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::OrderBuilds(cyclus::Commodity& commodity,
                               double unmetdemand) {
  using std::vector;
  vector<cyclus::BuildOrder> orders =
    buildmanager_.MakeBuildDecision(commodity, unmetdemand);

  LOG(cyclus::LEV_INFO3, "greg") << "The build orders have been determined. "
                                 << orders.size()
                                 << " different type(s) of prototypes will be built.";

  for (int i = 0; i < orders.size(); i++) {
    cyclus::BuildOrder order = orders.at(i);
    cyclus::InstModel* instcast = dynamic_cast<cyclus::InstModel*>(order.builder);
    cyclus::Model* modelcast = dynamic_cast<cyclus::Model*>(order.producer);
    if (!instcast || !modelcast) {
      throw cyclus::CastError("growth_region.has tried to incorrectly cast an already known entity.");
    }

    LOG(cyclus::LEV_INFO3, "greg") << "A build order for " << order.number
                                   << " prototype(s) of type "
                                   << dynamic_cast<cyclus::Model*>(modelcast)->name()
                                   << " from builder " << instcast->name()
                                   << " is being placed.";

    for (int j = 0; j < order.number; j++) {
      LOG(cyclus::LEV_DEBUG2, "greg") << "Ordering build number: " << j + 1;
      instcast->Build(modelcast->name());
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructGrowthRegion(cyclus::Context* ctx) {
  return new GrowthRegion(ctx);
}

} // namespace cycamore
