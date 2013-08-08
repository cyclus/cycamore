// manager_inst.cc
// Implements the ManagerInst class

#include "manager_inst.h"
#include "prototype.h"
#include "logger.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::RegisterAvailablePrototype(cyclus::Prototype* prototype) {
  cyclus::supply_demand::CommodityProducer* cast =
    dynamic_cast<cyclus::supply_demand::CommodityProducer*>(prototype);
  if (cast) {
    cyclus::action_building::Builder::RegisterProducer(cast);
    LOG(cyclus::LEV_DEBUG3, "maninst") << "ManagerInst " << name()
                                       << " has registered a producer prototype: "
                                       << dynamic_cast<cyclus::Model*>(prototype)->name()
                                       << " and "
                                       << " now has " << NBuildingPrototypes()
                                       << " registered total.";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::RegisterCloneAsBuilt(cyclus::Prototype* clone) {
  cyclus::supply_demand::CommodityProducer* cast =
    dynamic_cast<cyclus::supply_demand::CommodityProducer*>(clone);
  if (cast) {
    cyclus::supply_demand::CommodityProducerManager::RegisterProducer(cast);
    if (cyclus::LEV_DEBUG3 >= cyclus::Logger::ReportLevel()) {
      LOG(cyclus::LEV_DEBUG3, "maninst") << "ManagerInst " << name()
                                         << " has registered a producer clone:";
      WriteProducerInformation(cast);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::RegisterCloneAsDecommissioned(cyclus::Prototype* clone) {
  cyclus::supply_demand::CommodityProducer* cast =
    dynamic_cast<cyclus::supply_demand::CommodityProducer*>(clone);
  if (cast) {
    cyclus::supply_demand::CommodityProducerManager::UnRegisterProducer(cast);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::WriteProducerInformation(
  cyclus::supply_demand::CommodityProducer* producer) {
  set<cyclus::Commodity, cyclus::CommodityCompare> commodities =
    producer->ProducedCommodities();
  set<cyclus::Commodity, cyclus::CommodityCompare>::iterator it;

  LOG(cyclus::LEV_DEBUG3, "maninst") << " Clone produces " << commodities.size()
                                     << " commodities.";
  for (it = commodities.begin(); it != commodities.end(); it++) {
    LOG(cyclus::LEV_DEBUG3, "maninst") << " Commodity produced: " << it->name();
    LOG(cyclus::LEV_DEBUG3, "maninst") << "           capacity: " <<
                                       producer->ProductionCapacity(*it);
    LOG(cyclus::LEV_DEBUG3, "maninst") << "               cost: " <<
                                       producer->ProductionCost(*it);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructManagerInst() {
  return new ManagerInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructManagerInst(cyclus::Model* model) {
  delete model;
}



