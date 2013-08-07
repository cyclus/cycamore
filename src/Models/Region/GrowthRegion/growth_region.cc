// growth_region.cc
// Implements the GrowthRegion class

#include "growth_region.h"

#include "QueryEngine.h"
#include "SymbolicFunctionFactories.h"
#include "InstModel.h"
#include "Prototype.h"
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
void GrowthRegion::initModuleMembers(cyclus::QueryEngine* qe) {
  LOG(cyclus::LEV_DEBUG2, "greg") << "A Growth Region is being initialized";
  
  string query = "commodity";

  int nCommodities = qe->nElementsMatchingQuery(query);

  // populate supply demand manager info for each commodity
  for (int i=0; i<nCommodities; i++) {
    addCommodityDemand(qe->queryElement(query,i));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::addCommodityDemand(cyclus::QueryEngine* qe) 
{
  // instantiate product
  string name = qe->getElementContent("name");
  cyclus::Commodity commodity(name);
  registerCommodity(commodity);

  // instantiate demand
  string query = "demand";
  int n = qe->nElementsMatchingQuery(query);
  cyclus::PiecewiseFunctionFactory pff;

  for (int i = 0; i < n; i++)
    {
      cyclus::QueryEngine* demand = qe->queryElement(query,i);

      string type = demand->getElementContent("type");
      string params = demand->getElementContent("parameters");
      int time;
      try 
        {
          time = lexical_cast<int>(demand->getElementContent("start_time"));
        }
      catch (cyclus::Error e) 
        {
          time = 0;
        }

      cyclus::BasicFunctionFactory bff;
      bool continuous = (i != 0); // the first entry is not continuous
      pff.addFunction(bff.getFunctionPtr(type,params),time,continuous);
    }

  // register the commodity and demand
  sdmanager_.registerCommodity(commodity,pff.getFunctionPtr());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::enterSimulationAsModule() 
{
  for (int i = 0; i != nChildren(); i++) 
    {
      cyclus::Model* child = children(i);  
      registerCommodityProducerManager(child);
      registerBuilder(child);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::handleTick(int time) 
{
  set<cyclus::Commodity>::iterator it;
  for (it = commodities_.begin(); it != commodities_.end(); it++)
    {
      cyclus::Commodity commodity = *it;
      double demand = sdmanager_.demand(commodity,time);
      double supply = sdmanager_.supply(commodity);
      double unmet_demand = demand - supply;

      LOG(cyclus::LEV_INFO3,"greg") << "GrowthRegion: " << name() 
                            << " at time: " << time 
                            << " has the following values regaring "
                            << " commodity: " << commodity.name();
      LOG(cyclus::LEV_INFO3,"greg") << "  * demand = " << demand;
      LOG(cyclus::LEV_INFO3,"greg") << "  * supply = " << supply;
      LOG(cyclus::LEV_INFO3,"greg") << "  * unmet demand = " << unmet_demand;
      
      if (unmet_demand > 0)
        {
          orderBuilds(commodity,unmet_demand);
        }
    }
  cyclus::RegionModel::handleTick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerCommodity(cyclus::Commodity& commodity) 
{
  if (commodities_.find(commodity) != commodities_.end())
    {
      throw cyclus::KeyError("A GrowthRegion ("
                                           + name() + " is trying to register a commodity twice.");
    }
  else
    {
      commodities_.insert(commodity);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerCommodityProducerManager(cyclus::Model* child)
{
  cyclus::SupplyDemand::CommodityProducerManager* cast = dynamic_cast<cyclus::SupplyDemand::CommodityProducerManager*>(child);
  if (cast)
    {
      sdmanager_.registerProducerManager(cast);
    }
} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::registerBuilder(cyclus::Model* child)
{
  cyclus::ActionBuilding::Builder* cast = dynamic_cast<cyclus::ActionBuilding::Builder*>(child);
  if (cast)
    {
      buildmanager_.registerBuilder(cast);
    }
} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GrowthRegion::orderBuilds(cyclus::Commodity& commodity, double unmet_demand)
{
  vector<cyclus::ActionBuilding::BuildOrder> orders = buildmanager_.makeBuildDecision(commodity,unmet_demand);
  
  LOG(cyclus::LEV_INFO3,"greg") << "The build orders have been determined. " 
                        << orders.size() 
                        << " different type(s) of prototypes will be built.";

  for (int i = 0; i < orders.size(); i++)
    {
      cyclus::ActionBuilding::BuildOrder order = orders.at(i);
      cyclus::InstModel* instcast = dynamic_cast<cyclus::InstModel*>(order.builder);
      cyclus::Prototype* protocast = dynamic_cast<cyclus::Prototype*>(order.producer);
      if(instcast && protocast)
        {
          LOG(cyclus::LEV_INFO3,"greg") << "A build order for " << order.number
                                << " prototype(s) of type " 
                                << dynamic_cast<cyclus::Model*>(protocast)->name()
                                << " from builder " << instcast->name()
                                << " is being placed.";

          for (int j = 0; j < order.number; j++)
            {
              LOG(cyclus::LEV_DEBUG2,"greg") << "Ordering build number: " << j+1; 
              instcast->build(protocast);
            }
        }
      else
        {
          throw cyclus::CastError("growth_region.has tried to incorrectly cast an already known entity.");
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructGrowthRegion() 
{
  return new GrowthRegion();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructGrowthRegion(cyclus::Model* model) 
{
      delete model;
}

