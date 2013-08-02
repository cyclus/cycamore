// ManagerInst.cpp
// Implements the ManagerInst class

#include "ManagerInst.h"
#include "Prototype.h"
#include "Logger.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void ManagerInst::registerAvailablePrototype(cyclus::Prototype* prototype) 
{
  cyclus::SupplyDemand::CommodityProducer* cast = dynamic_cast<cyclus::SupplyDemand::CommodityProducer*>(prototype);
  if (cast) 
    {
      cyclus::ActionBuilding::Builder::registerProducer(cast);
      LOG(cyclus::LEV_DEBUG3,"maninst") << "ManagerInst " << name() 
                                << " has registered a producer prototype: "
                                << dynamic_cast<cyclus::Model*>(prototype)->name()
                                << " and "
                                <<" now has " << nBuildingPrototypes()
                                << " registered total.";
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::registerCloneAsBuilt(cyclus::Prototype* clone)
{
  cyclus::SupplyDemand::CommodityProducer* cast = dynamic_cast<cyclus::SupplyDemand::CommodityProducer*>(clone);
  if (cast) 
    {
      cyclus::SupplyDemand::CommodityProducerManager::registerProducer(cast);
      if (cyclus::LEV_DEBUG3 >= cyclus::Logger::ReportLevel())
        {
          LOG(cyclus::LEV_DEBUG3,"maninst") << "ManagerInst " << name() 
                                    << " has registered a producer clone:";
          writeProducerInformation(cast);
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::registerCloneAsDecommissioned(cyclus::Prototype* clone)
{
  cyclus::SupplyDemand::CommodityProducer* cast = dynamic_cast<cyclus::SupplyDemand::CommodityProducer*>(clone);
  if (cast) 
    {
      cyclus::SupplyDemand::CommodityProducerManager::unRegisterProducer(cast);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::writeProducerInformation(cyclus::SupplyDemand::CommodityProducer* producer)
{
  set<cyclus::Commodity,cyclus::CommodityCompare> commodities = producer->producedCommodities();
  set<cyclus::Commodity,cyclus::CommodityCompare>::iterator it;

  LOG(cyclus::LEV_DEBUG3,"maninst") << " Clone produces " << commodities.size() << " commodities.";
  for (it = commodities.begin(); it != commodities.end(); it++)
    {
      LOG(cyclus::LEV_DEBUG3,"maninst") << " Commodity produced: " << it->name();
      LOG(cyclus::LEV_DEBUG3,"maninst") << "           capacity: " << producer->productionCapacity(*it);
      LOG(cyclus::LEV_DEBUG3,"maninst") << "               cost: " << producer->productionCost(*it);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructManagerInst() 
{
  return new ManagerInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructManagerInst(cyclus::Model* model) 
{
  delete model;
}



