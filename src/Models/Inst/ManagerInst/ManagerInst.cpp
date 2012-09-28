// ManagerInst.cpp
// Implements the ManagerInst class

#include "ManagerInst.h"
#include "Prototype.h"

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
    }  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::registerCloneAsBuilt(Prototype* clone)
{
  CommodityProducer* cast = dynamic_cast<CommodityProducer*>(clone);
  if (cast) 
    {
      CommodityProducerManager::registerProducer(cast);
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



