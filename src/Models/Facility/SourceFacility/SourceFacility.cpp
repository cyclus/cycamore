// SourceFacility.cpp
// Implements the SourceFacility class
#include "SourceFacility.h"

#include "QueryEngine.h"
#include "Logger.h"
#include "RecipeLibrary.h"
#include "GenericResource.h"
#include "CycException.h"
#include "CycLimits.h"
#include "MarketModel.h"

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
  ordersWaiting_ = deque<cyclus::msg_ptr>();
  inventory_ = cyclus::MatBuff();
  setMaxInventorySize(numeric_limits<double>::max());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
SourceFacility::~SourceFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::initModuleMembers(cyclus::QueryEngine* qe) {
  cyclus::QueryEngine* output = qe->queryElement("output");

  setRecipe(output->getElementContent("recipe"));

  string data = output->getElementContent("outcommodity");
  setCommodity(data);
  cyclus::Commodity commod(data);
  cyclus::SupplyDemand::CommodityProducer::addCommodity(commod);
  
  double val = numeric_limits<double>::max();
  try
  {
    data = output->getElementContent("output_capacity"); 
    val = lexical_cast<double>(data); // overwrite default if given a value
  }
  catch (cyclus::CycNullQueryException e) {}
  cyclus::SupplyDemand::CommodityProducer::setCapacity(commod, val);
  setCapacity(val);  

  try
    {
      data = output->getElementContent("inventorysize"); 
      setMaxInventorySize(lexical_cast<double>(data));
    } 
  catch (cyclus::CycNullQueryException e) 
    {
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
void SourceFacility::cloneModuleMembersFrom(cyclus::FacilityModel* sourceModel) {
  SourceFacility* source = dynamic_cast<SourceFacility*>(sourceModel);
  setCommodity(source->commodity());
  setCapacity(source->capacity());
  setRecipe(source->recipe());
  setMaxInventorySize(source->maxInventorySize());
  copyProducedCommoditiesFrom(source);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::handleTick(int time){
  LOG(cyclus::LEV_INFO3, "SrcFac") << facName() << " is ticking {";

  generateMaterial();
  cyclus::Transaction trans = buildTransaction();

  LOG(cyclus::LEV_INFO4, "SrcFac") << "offers "<< trans.resource()->quantity() << " kg of "
                           << out_commod_ << ".";

  sendOffer(trans);

  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::handleTock(int time){
  LOG(cyclus::LEV_INFO3, "SrcFac") << facName() << " is tocking {";

  // check what orders are waiting,
  // send material if you have it now
  while (!ordersWaiting_.empty()) {
    cyclus::Transaction order = ordersWaiting_.front()->trans();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Order is for: " << order.resource()->quantity();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Inventory is: " << inventory_.quantity();
    if (order.resource()->quantity() - inventory_.quantity() > cyclus::eps()) {
      LOG(cyclus::LEV_INFO3, "SrcFac") << "Not enough inventory. Waitlisting remaining orders.";
      break;
    } else {
      LOG(cyclus::LEV_INFO3, "SrcFac") << "Satisfying order.";
      order.approveTransfer();
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
void SourceFacility::receiveMessage(cyclus::msg_ptr msg) {
  // is this a message from on high? 
  if(msg->trans().supplier() == this) {
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(cyclus::LEV_INFO5, "SrcFac") << name() << " just received an order.";
    LOG(cyclus::LEV_INFO5, "SrcFac") << "for " << msg->trans().resource()->quantity() 
                             << " of " << msg->trans().commod();
  } else {
    throw cyclus::CycException("SourceFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vector<cyclus::rsrc_ptr> SourceFacility::removeResource(cyclus::Transaction order) {
  return cyclus::MatBuff::toRes(inventory_.popQty(order.resource()->quantity()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::setCommodity(std::string name) {
  out_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string SourceFacility::commodity() {
  return out_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::setCapacity(double capacity) {
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
  inventory_.setCapacity(size);
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

  cyclus::mat_rsrc_ptr newMat = 
    cyclus::mat_rsrc_ptr(new cyclus::Material(cyclus::RecipeLibrary::Recipe(recipe_name_)));
  double amt = capacity_;
  if (amt <= empty_space) {
    newMat->setQuantity(amt); // plenty of room
  } else {
    newMat->setQuantity(empty_space); // not enough room
  }
  inventory_.pushOne(newMat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::Transaction SourceFacility::buildTransaction() {
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  cyclus::mat_rsrc_ptr trade_res = cyclus::mat_rsrc_ptr(new cyclus::Material(cyclus::RecipeLibrary::Recipe(recipe())));
  trade_res->setQuantity(offer_amt);

  cyclus::Transaction trans(this, cyclus::OFFER);
  trans.setCommod(out_commod_);
  trans.setMinFrac(min_amt/offer_amt);
  trans.setPrice(commod_price_);
  trans.setResource(trade_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::sendOffer(cyclus::Transaction trans) {
  cyclus::MarketModel* market = cyclus::MarketModel::marketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  cyclus::msg_ptr msg(new cyclus::Message(this, recipient, trans)); 
  msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" cyclus::Model* constructSourceFacility() {
  return new SourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructSourceFacility(cyclus::Model* model) {
  delete model;
}
