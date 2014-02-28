// manager_inst.cc
// Implements the ManagerInst class

#include "manager_inst.h"
#include "logger.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst(cyclus::Context* ctx)
    : cyclus::InstModel(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::RegisterAvailablePrototype(std::string prototype) {
  using cyclus::CommodityProducer;
  try {
    CommodityProducer* cast = context()->CreateModel<CommodityProducer>(prototype);
    cyclus::Builder::RegisterProducer(cast);
    LOG(cyclus::LEV_DEBUG3, "maninst") << "ManagerInst " << name()
                                       << " has registered a producer prototype: "
                                       << prototype
                                       << " and "
                                       << " now has " << NBuildingPrototypes()
                                       << " registered total.";
  } catch (cyclus::CastError err) {}
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::BuildNotify(cyclus::Model* clone) {
  cyclus::CommodityProducer* cast =
    dynamic_cast<cyclus::CommodityProducer*>(clone);
  if (cast) {
    cyclus::CommodityProducerManager::RegisterProducer(cast);
    if (cyclus::LEV_DEBUG3 >= cyclus::Logger::ReportLevel()) {
      LOG(cyclus::LEV_DEBUG3, "maninst") << "ManagerInst " << name()
                                         << " has registered a producer clone:";
      WriteProducerInformation(cast);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::DecomNotify(cyclus::Model* clone) {
  cyclus::CommodityProducer* cast =
    dynamic_cast<cyclus::CommodityProducer*>(clone);
  if (cast) {
    cyclus::CommodityProducerManager::UnRegisterProducer(cast);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::WriteProducerInformation(
  cyclus::CommodityProducer* producer) {
  using std::set;
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
extern "C" cyclus::Model* ConstructManagerInst(cyclus::Context* ctx) {
  return new ManagerInst(ctx);
}
} // namespace cycamore


