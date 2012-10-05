// ManagerInst.cpp
// Implements the ManagerInst class

#include "ManagerInst.h"
#include "Prototype.h"
#include "Logger.h"

using namespace std;
using namespace SupplyDemand;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void ManagerInst::registerAvailablePrototype(Prototype* prototype) 
{
  CommodityProducer* cast = dynamic_cast<CommodityProducer*>(prototype);
  if (cast) 
    {
      Builder::registerProducer(cast);
      LOG(LEV_DEBUG3,"maninst") << "ManagerInst " << name() 
                                << " has registered a producer prototype: "
                                << dynamic_cast<Model*>(prototype)->name()
                                << " and "
                                <<" now has " << nBuildingPrototypes()
                                << " registered total.";
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::registerCloneAsBuilt(Prototype* clone)
{
  CommodityProducer* cast = dynamic_cast<CommodityProducer*>(clone);
  if (cast) 
    {
      CommodityProducerManager::registerProducer(cast);
      if (LEV_DEBUG3 >= Logger::ReportLevel())
        {
          LOG(LEV_DEBUG3,"maninst") << "ManagerInst " << name() 
                                    << " has registered a producer clone:";
          writeProducerInformation(cast);
        }
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::registerCloneAsDecommissioned(Prototype* clone)
{
  CommodityProducer* cast = dynamic_cast<CommodityProducer*>(clone);
  if (cast) 
    {
      CommodityProducerManager::unRegisterProducer(cast);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::writeProducerInformation(SupplyDemand::CommodityProducer* producer)
{
  set<Commodity,CommodityCompare> commodities = producer->producedCommodities();
  set<Commodity,CommodityCompare>::iterator it;

  LOG(LEV_DEBUG3,"maninst") << " Clone produces " << commodities.size() << " commodities.";
  for (it = commodities.begin(); it != commodities.end(); it++)
    {
      LOG(LEV_DEBUG3,"maninst") << " Commodity produced: " << it->name();
      LOG(LEV_DEBUG3,"maninst") << "           capacity: " << producer->productionCapacity(*it);
      LOG(LEV_DEBUG3,"maninst") << "               cost: " << producer->productionCost(*it);
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructManagerInst() 
{
  return new ManagerInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructManagerInst(Model* model) 
{
  delete model;
}



