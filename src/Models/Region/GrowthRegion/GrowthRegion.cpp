// GrowthRegion.cpp
// Implements the GrowthRegion class

#include "GrowthRegion.h"

#include "QueryEngine.h"
#include "SymbolicFunctionFactories.h"
#include "InstModel.h"
#include "Prototype.h"
#include "CycException.h"

#include <vector>

using namespace std;
using namespace SupplyDemand;
using namespace ActionBuilding;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
GrowthRegion::GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
GrowthRegion::~GrowthRegion() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::initModuleMembers(QueryEngine* qe) {
  LOG(LEV_DEBUG2, "greg") << "A Growth Region is being initialized";
  
  string query = "commodity";

  int nCommodities = qe->nElementsMatchingQuery(query);

  // populate supply demand manager info for each commodity
  for (int i=0; i<nCommodities; i++) {
    addCommodityDemand(qe->queryElement(query,i));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::addCommodityDemand(QueryEngine* qe) 
{
  // instantiate product
  string name = qe->getElementContent("name");
  Commodity commodity(name);
  registerCommodity(commodity);

  // instantiate demand
  QueryEngine* demand = qe->queryElement("demand");
  string type = demand->getElementContent("type");
  string params = demand->getElementContent("parameters");
  BasicFunctionFactory bff;
  FunctionPtr demand_function = bff.getFunctionPtr(type,params);

  // register the commodity and demand
  sdmanager_.registerCommodity(commodity,demand_function);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::enterSimulationAsModule() 
{
  for (int i = 0; i != nChildren(); i++) 
    {
      Model* child = children(i);  
      registerCommodityProducerManager(child);
      registerBuilder(child);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::handleTick(int time) 
{
  set<Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); it++)
    {
      Commodity commodity = *it;
      double demand = sdmanager_.demand(commodity,time);
      double supply = sdmanager_.supply(commodity);
      double unmet_demand = demand - supply;

      LOG(LEV_INFO3,"greg") << "GrowthRegion: " << name() 
                            << " at time: " << time 
                            << " has the following values regaring "
                            << " commodity: " << commodity.name();
      LOG(LEV_INFO3,"greg") << "  * demand = " << demand;
      LOG(LEV_INFO3,"greg") << "  * supply = " << supply;
      LOG(LEV_INFO3,"greg") << "  * unmet demand = " << unmet_demand;
      
      if (unmet_demand > 0)
        {
          orderBuilds(commodity,unmet_demand);
        }
    }
  RegionModel::handleTick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerCommodity(Commodity& commodity) 
{
  if (commodities_.find(commodity) != commodities_.end())
    {
      throw CycDoubleRegistrationException("A GrowthRegion ("
                                           + name() + " is trying to register a commodity twice.");
    }
  else
    {
      commodities_.insert(commodity);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerCommodityProducerManager(Model* child)
{
  CommodityProducerManager* cast = dynamic_cast<CommodityProducerManager*>(child);
  if (cast)
    {
      sdmanager_.registerProducerManager(cast);
    }
} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerBuilder(Model* child)
{
  Builder* cast = dynamic_cast<Builder*>(child);
  if (cast)
    {
      buildmanager_.registerBuilder(cast);
    }
} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::orderBuilds(Commodity& commodity, double unmet_demand)
{
  vector<BuildOrder> orders = buildmanager_.makeBuildDecision(commodity,unmet_demand);
  
  LOG(LEV_INFO3,"greg") << "The build orders have been determined. " 
                        << orders.size() 
                        << " different type(s) of prototypes will be built.";

  for (int i = 0; i < orders.size(); i++)
    {
      BuildOrder order = orders.at(i);
      InstModel* instcast = dynamic_cast<InstModel*>(order.builder);
      Prototype* protocast = dynamic_cast<Prototype*>(order.producer);
      if(instcast && protocast)
        {
          LOG(LEV_INFO3,"greg") << "A build order for " << order.number
                                << " prototype(s) of type " 
                                << dynamic_cast<Model*>(protocast)->name()
                                << " from builder " << instcast->name()
                                << " is being placed.";

          for (int j = 0; j < order.number; j++)
            {
              LOG(LEV_DEBUG3,"greg") << "Ordering build number: " << j+1; 
              instcast->build(protocast);
            }
        }
      else
        {
          throw CycOverrideException("GrowthRegion has tried to incorrectly cast an already known entity.");
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructGrowthRegion() 
{
  return new GrowthRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructGrowthRegion(Model* model) 
{
      delete model;
}

