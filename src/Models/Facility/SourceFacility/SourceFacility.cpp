// SourceFacility.cpp
// Implements the SourceFacility class
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "SourceFacility.h"

#include "Logger.h"
#include "RecipeLibrary.h"
#include "GenericResource.h"
#include "CycException.h"
#include "MarketModel.h"

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
SourceFacility::SourceFacility() : 
  out_commod_(""), 
  recipe_name_(""), 
  commod_price_(0), 
  capacity_(numeric_limits<double>::max()) 
{
  ordersWaiting_ = deque<msg_ptr>();
  inventory_ = MatBuff();
  setMaxInventorySize(numeric_limits<double>::max());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
SourceFacility::~SourceFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::initModuleMembers(QueryEngine* qe) {
  QueryEngine* output = qe->queryElement("output");

  setCommodity(output->getElementContent("outcommodity"));
  setRecipe(output->getElementContent("recipe"));

  string data;
  data = output->getElementContent("output_capacity"); 
  setCapacity(lexical_cast<double>(data));  
  data = output->getElementContent("inventorysize"); 
  setMaxInventorySize(lexical_cast<double>(data));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '" 
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step "
     << " with max inventory of " << inventory_.capacity() << " kg.";
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Prototype* SourceFacility::clone() {
  SourceFacility* clone = new SourceFacility();
  clone->setCommodity(commodity());
  clone->setCapacity(capacity());
  clone->setRecipe(recipe());
  clone->setMaxInventorySize(maxInventorySize());
  return clone;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::handleTick(int time){
  LOG(LEV_INFO3, "SrcFac") << facName() << " is ticking {";

  generateMaterial();
  Transaction trans = buildTransaction();

  LOG(LEV_INFO4, "SrcFac") << "offers "<< trans.resource()->quantity() << " kg of "
                           << out_commod_ << ".";

  sendOffer(trans);

  LOG(LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::handleTock(int time){
  LOG(LEV_INFO3, "SrcFac") << facName() << " is tocking {";

  // check what orders are waiting,
  // send material if you have it now
  while (!ordersWaiting_.empty()) {
    Transaction order = ordersWaiting_.front()->trans();
    if (order.resource()->quantity() - inventory_.quantity() > EPS_KG) {
      LOG(LEV_INFO3, "SrcFac") << "Not enough inventory. Waitlisting remaining orders.";
      break;
    } else {
      order.approveTransfer();
      ordersWaiting_.pop_front();
    }
  }
  // For now, lets just print out what we have at each timestep.
  LOG(LEV_INFO4, "SrcFac") << "SourceFacility " << this->ID()
                  << " is holding " << this->inventory_.quantity()
                  << " units of material at the close of month " << time
                  << ".";

  LOG(LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::receiveMessage(msg_ptr msg){

  // is this a message from on high? 
  if(msg->trans().supplier() == this){
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(LEV_INFO5, "SrcFac") << name() << " just received an order.";
  } else {
    throw CycException("SourceFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vector<rsrc_ptr> SourceFacility::removeResource(Transaction order) {
  return MatBuff::toRes(inventory_.popQty(order.resource()->quantity()));
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
  if (empty_space < EPS_KG) {
    return; // no room
  }

  mat_rsrc_ptr newMat = 
    mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(recipe_name_)));
  double amt = capacity_;
  if (amt <= empty_space) {
    newMat->setQuantity(amt); // plenty of room
  } else {
    newMat->setQuantity(empty_space); // not enough room
  }
  inventory_.pushOne(newMat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Transaction SourceFacility::buildTransaction() {
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  gen_rsrc_ptr offer_res = 
    gen_rsrc_ptr(new GenericResource(out_commod_,"kg",offer_amt));

  Transaction trans(this, OFFER);
  trans.setCommod(out_commod_);
  trans.setMinFrac(min_amt/offer_amt);
  trans.setPrice(commod_price_);
  trans.setResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SourceFacility::sendOffer(Transaction trans) {
  MarketModel* market = MarketModel::marketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  msg_ptr msg(new Message(this, recipient, trans)); 
  msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" Model* constructSourceFacility() {
  return new SourceFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructSourceFacility(Model* model) {
      delete model;
}

