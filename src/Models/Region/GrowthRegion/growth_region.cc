// growth_region.cc
// Implements the GrowthRegion class

#include "growth_region.h"

#include "query_engine.h"
#include "symbolic_function_factories.h"
#include "inst_model.h"
#include "prototype.h"
#include "error.h"

#include <vector>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GrowthRegion::~GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::InitModuleMembers(cyclus::QueryEngine* qe) {
  LOG(cyclus::LEV_DEBUG2, "greg") << "A Growth Region is being initialized";

  string query = "commodity";

  int nCommodities = qe->NElementsMatchingQuery(query);

  // populate supply demand manager info for each commodity
  for (int i = 0; i < nCommodities; i++) {
    AddCommodityDemand(qe->QueryElement(query, i));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::AddCommodityDemand(cyclus::QueryEngine* qe) {
  // instantiate product
  string name = qe->GetElementContent("name");
  cyclus::Commodity commodity(name);
  RegisterCommodity(commodity);

  // instantiatedemand
  string query = "demand";
  int n = qe->NElementsMatchingQuery(query);
  cyclus::PiecewiseFunctionFactory pff;

  for (int i = 0; i < n; i++) {
    cyclus::QueryEngine* demand = qe->QueryElement(query, i);

    string type = demand->GetElementContent("type");
    string params = demand->GetElementContent("parameters");
    int time;
    try {
      time = lexical_cast<int>(demand->GetElementContent("start_time"));
    } catch (cyclus::Error e) {
      time = 0;
    }

    cyclus::BasicFunctionFactory bff;
    bool continuous = (i != 0); // the first entry is not continuous
    pff.AddFunction(bff.GetFunctionPtr(type, params), time, continuous);
  }

  // register the commodity anddemand
  sdmanager_.RegisterCommodity(commodity, pff.GetFunctionPtr());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::EnterSimulationAsModule() {
  for (int i = 0; i != NChildren(); i++) {
    cyclus::Model* child = children(i);
    RegisterCommodityProducerManager(child);
    RegisterBuilder(child);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::HandleTick(int time) {
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
      orderBuilds(commodity, unmetdemand);
    }
  }
  cyclus::RegionModel::HandleTick(time);
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
  cyclus::supply_demand::CommodityProducerManager* cast =
    dynamic_cast<cyclus::supply_demand::CommodityProducerManager*>(child);
  if (cast) {
    sdmanager_.RegisterProducerManager(cast);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::RegisterBuilder(cyclus::Model* child) {
  cyclus::action_building::Builder* cast =
    dynamic_cast<cyclus::action_building::Builder*>(child);
  if (cast) {
    buildmanager_.RegisterBuilder(cast);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::orderBuilds(cyclus::Commodity& commodity,
                               double unmetdemand) {
  vector<cyclus::action_building::BuildOrder> orders =
    buildmanager_.MakeBuildDecision(commodity, unmetdemand);

  LOG(cyclus::LEV_INFO3, "greg") << "The build orders have been determined. "
                                 << orders.size()
                                 << " different type(s) of prototypes will be built.";

  for (int i = 0; i < orders.size(); i++) {
    cyclus::action_building::BuildOrder order = orders.at(i);
    cyclus::InstModel* instcast = dynamic_cast<cyclus::InstModel*>(order.builder);
    cyclus::Prototype* protocast = dynamic_cast<cyclus::Prototype*>(order.producer);
    if (instcast && protocast) {
      LOG(cyclus::LEV_INFO3, "greg") << "A build order for " << order.number
                                     << " prototype(s) of type "
                                     << dynamic_cast<cyclus::Model*>(protocast)->name()
                                     << " from builder " << instcast->name()
                                     << " is being placed.";

      for (int j = 0; j < order.number; j++) {
        LOG(cyclus::LEV_DEBUG2, "greg") << "Ordering build number: " << j + 1;
        instcast->Build(protocast);
      }
    } else {
      throw cyclus::CastError("growth_region.has tried to incorrectly cast an already known entity.");
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructGrowthRegion() {
  return new GrowthRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructGrowthRegion(cyclus::Model* model) {
  delete model;
}

