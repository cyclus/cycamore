// source_facility.cc
// Implements the SourceFacility class
#include "source_facility.h"

#include "query_engine.h"
#include "logger.h"
#include "recipe_library.h"
#include "generic_resource.h"
#include "error.h"
#include "cyc_limits.h"
#include "market_model.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility() :
  out_commod_(""),
  recipe_name_(""),
  commod_price_(0),
  capacity_(numeric_limits<double>::max()) {
  ordersWaiting_ = deque<cyclus::Message::Ptr>();
  inventory_ = cyclus::MatBuff();
  setMaxInventorySize(numeric_limits<double>::max());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::~SourceFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  cyclus::QueryEngine* output = qe->QueryElement("output");

  setRecipe(output->GetElementContent("recipe"));

  string data = output->GetElementContent("outcommodity");
  SetCommodity(data);
  cyclus::Commodity commod(data);
  cyclus::supply_demand::CommodityProducer::AddCommodity(commod);

  double val = numeric_limits<double>::max();
  try {
    data = output->GetElementContent("output_capacity");
    val = lexical_cast<double>(data); // overwrite default if given a value
  } catch (cyclus::Error e) {}
  cyclus::supply_demand::CommodityProducer::SetCapacity(commod, val);
  SetCapacity(val);

  try {
    data = output->GetElementContent("inventorysize");
    setMaxInventorySize(lexical_cast<double>(data));
  } catch (cyclus::Error e) {
    setMaxInventorySize(numeric_limits<double>::max());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '"
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step "
     << " with max inventory of " << inventory_.capacity() << " kg.";
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::CloneModuleMembersFrom(cyclus::FacilityModel*
                                            sourceModel) {
  SourceFacility* source = dynamic_cast<SourceFacility*>(sourceModel);
  SetCommodity(source->commodity());
  SetCapacity(source->capacity());
  setRecipe(source->recipe());
  setMaxInventorySize(source->maxInventorySize());
  CopyProducedCommoditiesFrom(source);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is ticking {";

  generateMaterial();
  cyclus::Transaction trans = buildTransaction();

  LOG(cyclus::LEV_INFO4, "SrcFac") << "offers " << trans.resource()->quantity() <<
                                   " kg of "
                                   << out_commod_ << ".";

  sendOffer(trans);

  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is tocking {";

  // check what orders are waiting,
  // send material if you have it now
  while (!ordersWaiting_.empty()) {
    cyclus::Transaction order = ordersWaiting_.front()->trans();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Order is for: " <<
                                     order.resource()->quantity();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Inventory is: " << inventory_.quantity();
    if (order.resource()->quantity() - inventory_.quantity() > cyclus::eps()) {
      LOG(cyclus::LEV_INFO3, "SrcFac") <<
                                       "Not enough inventory. Waitlisting remaining orders.";
      break;
    } else {
      LOG(cyclus::LEV_INFO3, "SrcFac") << "Satisfying order.";
      order.ApproveTransfer();
      ordersWaiting_.pop_front();
    }
  }
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SrcFac") << "SourceFacility " << this->ID()
                                   << " is holding " << this->inventory_.quantity()
                                   << " units of material at the close of month " << time
                                   << ".";

  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::ReceiveMessage(cyclus::Message::Ptr msg) {
  if (msg->trans().supplier() == this) {
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(cyclus::LEV_INFO5, "SrcFac") << name() << " just received an order.";
    LOG(cyclus::LEV_INFO5, "SrcFac") << "for " <<
                                     msg->trans().resource()->quantity()
                                     << " of " << msg->trans().commod();
  } else {
    throw cyclus::Error("SourceFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
vector<cyclus::Resource::Ptr> SourceFacility::RemoveResource(
  cyclus::Transaction order) {
  return cyclus::MatBuff::ToRes(inventory_.PopQty(order.resource()->quantity()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetCommodity(std::string name) {
  out_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::commodity() {
  return out_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetCapacity(double capacity) {
  capacity_ = capacity;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::capacity() {
  return capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::setRecipe(std::string name) {
  recipe_name_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::recipe() {
  return recipe_name_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::setMaxInventorySize(double size) {
  inventory_.SetCapacity(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::maxInventorySize() {
  return inventory_.capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::inventorySize() {
  return inventory_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::generateMaterial() {

  double empty_space = inventory_.space();
  if (empty_space < cyclus::eps()) {
    return; // no room
  }

  cyclus::Material::Ptr newMat =
    cyclus::Material::Ptr(new cyclus::Material(cyclus::RecipeLibrary::Recipe(
                                                 recipe_name_)));
  double amt = capacity_;
  if (amt <= empty_space) {
    newMat->SetQuantity(amt); // plenty of room
  } else {
    newMat->SetQuantity(empty_space); // not enough room
  }
  inventory_.PushOne(newMat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Transaction SourceFacility::buildTransaction() {
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  cyclus::Material::Ptr trade_res = cyclus::Material::Ptr(new cyclus::Material(
                                                            cyclus::RecipeLibrary::Recipe(recipe())));
  trade_res->SetQuantity(offer_amt);

  cyclus::Transaction trans(this, cyclus::OFFER);
  trans.SetCommod(out_commod_);
  trans.SetMinFrac(min_amt / offer_amt);
  trans.SetPrice(commod_price_);
  trans.SetResource(trade_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::sendOffer(cyclus::Transaction trans) {
  cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  cyclus::Message::Ptr msg(new cyclus::Message(this, recipient, trans));
  msg->SendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructSourceFacility() {
  return new SourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructSourceFacility(cyclus::Model* model) {
  delete model;
}
