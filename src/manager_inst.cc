// Implements the ManagerInst class
#include "manager_inst.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::ManagerInst(cyclus::Context* ctx) : cyclus::Institution(ctx) { }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ManagerInst::~ManagerInst() {}

void ManagerInst::BuildNotify(Agent* a) {
  Register_(a);
}

void ManagerInst::DecomNotify(Agent* a) {
  Unregister_(a);
}

void ManagerInst::EnterNotify() {
  cyclus::Institution::EnterNotify();
  std::set<cyclus::Agent*>::iterator sit;
  for (sit = cyclus::Agent::children().begin();
       sit != cyclus::Agent::children().end();
       ++sit) {
    Agent* a = *sit;
    Register_(a);
  }

  using cyclus::toolkit::CommodityProducer;
  std::vector<std::string>::iterator vit;
  for (vit = prototypes.begin(); vit != prototypes.end(); ++vit) {
    Agent* a = context()->CreateAgent<Agent>(*vit);
    CommodityProducer* cp_cast = dynamic_cast<CommodityProducer*>(a);
    if (cp_cast != NULL) {
      LOG(cyclus::LEV_INFO3, "mani") << "Registering prototype "
                                     << a->prototype() << a->id()
                                     << " with the Builder interface.";
      Builder::Register(cp_cast);
    }
  }
}

void ManagerInst::Register_(Agent* a) {
  using cyclus::toolkit::CommodityProducer;
  using cyclus::toolkit::CommodityProducerManager;

  CommodityProducer* cp_cast = dynamic_cast<CommodityProducer*>(a);
  if (cp_cast != NULL) {
    LOG(cyclus::LEV_INFO3, "mani") << "Registering agent "
                                   << a->prototype() << a->id()
                                   << " as a commodity producer.";
    CommodityProducerManager::Register(cp_cast);
  }
}

void ManagerInst::Unregister_(Agent* a) {
  using cyclus::toolkit::CommodityProducer;
  using cyclus::toolkit::CommodityProducerManager;

  CommodityProducer* cp_cast = dynamic_cast<CommodityProducer*>(a);
  if (cp_cast != NULL)
    CommodityProducerManager::Unregister(cp_cast);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void ManagerInst::WriteProducerInformation(
  cyclus::toolkit::CommodityProducer* producer) {
  using std::set;
  set<cyclus::toolkit::Commodity,
      cyclus::toolkit::CommodityCompare> commodities =
          producer->ProducedCommodities();
  set<cyclus::toolkit::Commodity, cyclus::toolkit::CommodityCompare>::
      iterator it;

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructManagerInst(cyclus::Context* ctx) {
  return new ManagerInst(ctx);
}

}  // namespace cycamore
