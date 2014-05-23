// manager_inst.cc
// Implements the ManagerInst class

#include "manager_inst.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst(cyclus::Context* ctx)
    : cyclus::Institution(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the ManagerInst agent is experimental.");
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::BuildNotify(cyclus::Agent* clone) {
  cyclus::toolkit::CommodityProducer* cast =
    dynamic_cast<cyclus::toolkit::CommodityProducer*>(clone);
  if (cast) {
    cyclus::toolkit::CommodityProducerManager::Register(cast);
    if (cyclus::LEV_DEBUG3 >= cyclus::Logger::ReportLevel()) {
      LOG(cyclus::LEV_DEBUG3, "maninst") << "ManagerInst " << prototype()
                                         << " has registered a producer clone:";
      WriteProducerInformation(cast);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::DecomNotify(cyclus::Agent* clone) {
  cyclus::toolkit::CommodityProducer* cast =
    dynamic_cast<cyclus::toolkit::CommodityProducer*>(clone);
  if (cast) {
    cyclus::toolkit::CommodityProducerManager::Unregister(cast);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::WriteProducerInformation(
  cyclus::toolkit::CommodityProducer* producer) {
  using std::set;
  set<cyclus::toolkit::Commodity, cyclus::toolkit::CommodityCompare> commodities =
    producer->ProducedCommodities();
  set<cyclus::toolkit::Commodity, cyclus::toolkit::CommodityCompare>::iterator it;

  LOG(cyclus::LEV_DEBUG3, "maninst") << " Clone produces " << commodities.size()
                                     << " commodities.";
  for (it = commodities.begin(); it != commodities.end(); it++) {
    LOG(cyclus::LEV_DEBUG3, "maninst") << " Commodity produced: " << it->name();
    LOG(cyclus::LEV_DEBUG3, "maninst") << "           capacity: " <<
                                       producer->Capacity(*it);
    LOG(cyclus::LEV_DEBUG3, "maninst") << "               cost: " <<
                                       producer->Cost(*it);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructManagerInst(cyclus::Context* ctx) {
  return new ManagerInst(ctx);
}
} // namespace cycamore


