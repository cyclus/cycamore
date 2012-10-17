// EnrichmentFacility.cpp
// Implements the EnrichmentFacility class
#include "EnrichmentFacility.h"

#include "QueryEngine.h"
#include "Logger.h"
#include "CycException.h"
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::EnrichmentFacility() : 
{}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::~EnrichmentFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::initModuleMembers(QueryEngine* qe) 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::str() 
{
  std::stringstream ss;
  ss << FacilityModel::str()
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::cloneModuleMembersFrom(FacilityModel* sourceModel) 
{
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
  setCommodity(source->commodity());
  setCapacity(source->capacity());
  setRecipe(source->recipe());
  setMaxInventorySize(source->maxInventorySize());
  copyProducedCommoditiesFrom(source);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTick(int time){
  LOG(LEV_INFO3, "SrcFac") << facName() << " is ticking {";

  generateMaterial();
  Transaction trans = buildTransaction();

  LOG(LEV_INFO4, "SrcFac") << "offers "<< trans.resource()->quantity() << " kg of "
                           << out_commod_ << ".";

  sendOffer(trans);

  LOG(LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTock(int time){
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
  LOG(LEV_INFO4, "SrcFac") << "EnrichmentFacility " << this->ID()
                  << " is holding " << this->inventory_.quantity()
                  << " units of material at the close of month " << time
                  << ".";

  LOG(LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::receiveMessage(msg_ptr msg){

  // is this a message from on high? 
  if(msg->trans().supplier() == this){
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(LEV_INFO5, "SrcFac") << name() << " just received an order.";
  } else {
    throw CycException("EnrichmentFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vector<rsrc_ptr> EnrichmentFacility::removeResource(Transaction order) {
  return MatBuff::toRes(inventory_.popQty(order.resource()->quantity()));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::setCommodity(std::string name) {
  out_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::commodity() {
  return out_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::setCapacity(double capacity) {
  capacity_ = capacity;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double EnrichmentFacility::capacity() {
  return capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::setRecipe(std::string name) {
  recipe_name_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::recipe() {
  return recipe_name_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::setMaxInventorySize(double size) {
  inventory_.setCapacity(size);
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double EnrichmentFacility::maxInventorySize() {
  return inventory_.capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double EnrichmentFacility::inventorySize() {
  return inventory_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::generateMaterial() {

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
Transaction EnrichmentFacility::buildTransaction() {
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
void EnrichmentFacility::sendOffer(Transaction trans) 
{
  MarketModel* market = MarketModel::marketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  msg_ptr msg(new Message(this, recipient, trans)); 
  msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" Model* constructEnrichmentFacility() 
{
  return new EnrichmentFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructEnrichmentFacility(Model* model) 
{
      delete model;
}
