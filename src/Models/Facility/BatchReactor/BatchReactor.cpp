// BatchReactor.cpp
// Implements the BatchReactor class
#include "BatchReactor.h"

#include <sstream>
#include <cmath>

#include <boost/lexical_cast.hpp>

#include "Logger.h"
#include "GenericResource.h"
#include "RecipeLibrary.h"
#include "MarketModel.h"

using namespace std;
using boost::lexical_cast;
using namespace SupplyDemand;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
BatchReactor::BatchReactor() : 
  cycle_length_(1), 
  batches_per_core_(1), 
  core_loading_(1), 
  in_commod_(""), 
  in_recipe_(""), 
  out_commod_(""), 
  out_recipe_(""),
  cycle_timer_(1), 
  phase_(INIT)
{
  preCore_.makeUnlimited();
  inCore_.makeUnlimited();
  postCore_.makeUnlimited();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
BatchReactor::~BatchReactor() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::initModuleMembers(QueryEngine* qe) {
  QueryEngine* input = qe->queryElement("fuel_input");
  setInCommodity(input->getElementContent("incommodity"));
  setInRecipe(input->getElementContent("inrecipe"));

  QueryEngine* output = qe->queryElement("fuel_output");
  setOutCommodity(output->getElementContent("outcommodity"));
  setOutRecipe(output->getElementContent("outrecipe"));
  
  string data;
  data = qe->getElementContent("cyclelength"); 
  setCycleLength(lexical_cast<int>(data));
  data = qe->getElementContent("coreloading"); 
  setCoreLoading(lexical_cast<double>(data));  
  data = qe->getElementContent("batchespercore"); 
  setNBatches(lexical_cast<int>(data));

  QueryEngine* commodity = qe->queryElement("commodity_production");
  Commodity commod(commodity->getElementContent("commodity"));
  addCommodity(commod);
  data = commodity->getElementContent("capacity");
  CommodityProducer::setCapacity(commod,lexical_cast<double>(data));
  data = commodity->getElementContent("cost");
  CommodityProducer::setCost(commod,lexical_cast<double>(data));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string BatchReactor::str() { 
  std::stringstream ss;
  ss << FacilityModel::str();
  ss << " has facility parameters {"
     << ", Cycle Length = " << cycleLength()
     << ", Core Loading = " << coreLoading()
     << ", Batches Per Core = " << nBatches()
     << ", converts commodity '" << inCommodity()
     << "' into commodity '" << outCommodity()
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::cloneModuleMembersFrom(FacilityModel* sourceModel) {
  BatchReactor* source = dynamic_cast<BatchReactor*>(sourceModel);
  setCycleLength(source->cycleLength());
  setCoreLoading(source->coreLoading());
  setNBatches(source->nBatches());
  setInCommodity(source->inCommodity());
  setOutCommodity(source->outCommodity());
  setInRecipe(source->inRecipe());
  setOutRecipe(source->outRecipe());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::enterSimulationAsModule() {
  preCore_.setCapacity(coreLoading());
  inCore_.setCapacity(coreLoading());  
  resetCycleTimer();
  setPhase(BEGIN);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::handleTick(int time) {
  LOG(LEV_INFO3, "BReact") << name() << " is ticking at time " 
                           << time << " {";
  LOG(LEV_DEBUG3, "BReact") << "The current phase is: " << phase();


  if ( lifetimeReached() )
    setPhase(END);
  
  double fuel_quantity, request;
  string msg;

  switch( phase() ) {
  case INIT:
    // intentional fall through

  case OPERATION:
    break;
    
  case REFUEL:
    offloadBatch();

  case WAITING:
    // intentional fall through
    
  case BEGIN:
    fuel_quantity = preCore_.quantity() + inCore_.quantity();
    request = coreLoading() - fuel_quantity;
    makeRequest(request);
    break;
    
  case END:
    offloadCore();
    break;
    
  default:
    msg = "BatchReactors have undefined behvaior during ticks for phase: " 
      + phase();
    throw CycBatchReactorPhaseBehaviorException(msg);
    break;
  }
  
  makeOffers();

  LOG(LEV_INFO3, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::handleTock(int time) { 
  LOG(LEV_INFO3, "BReact") << name() << " is tocking {";
  LOG(LEV_DEBUG3, "BReact") << "The current phase is: " << phase();
  
  handleOrders();

  string msg;

  switch( phase() ) {
    
  case END:
    if ( postCore_.empty() )
      decommission();
    break;
    
  case BEGIN:
    // intentional fall through
    
  case WAITING:
    // intentional fall through
    
  case REFUEL:
    loadCore();
    if ( coreFilled() ) {
      setPhase(OPERATION);
    } else {
      setPhase(WAITING);
    }
    break; 
    
  case OPERATION:
    cycle_timer_++;
    if (cycleComplete())
      setPhase(REFUEL);
    break;
    
  default:
    msg = "BatchReactors have undefined behvaior during tocks for phase: " 
      + phase();
    throw CycBatchReactorPhaseBehaviorException(msg);
    break;
  }

  LOG(LEV_INFO3, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::receiveMessage(msg_ptr msg) {
  // is this a message from on high? 
  if(msg->trans().supplier()==this){
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(LEV_INFO5, "BReact") << name() << " just received an order.";
  }
  else {
    throw CycException("BatchReactor is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::sendMessage(Communicator* recipient, Transaction trans){
      msg_ptr msg(new Message(this, recipient, trans)); 
      msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vector<rsrc_ptr> BatchReactor::removeResource(Transaction order) {
  Transaction trans = order;
  double amt = trans.resource()->quantity();

  LOG(LEV_DEBUG4, "BReact") << "BatchReactor " << name() << " removed "
                            << amt << " of " << postCore_.quantity() 
                            << " to its postcore buffer.";
  
  return MatBuff::toRes(postCore_.popQty(amt));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::addResource(Transaction trans,
                               std::vector<rsrc_ptr> manifest) {
  double preQuantity = preCore_.quantity();
  preCore_.pushAll(MatBuff::toMat(manifest));
  double added = preCore_.quantity() - preQuantity;
  LOG(LEV_DEBUG4, "BReact") << "BatchReactor " << name() << " added "
                            << added << " to its precore buffer.";
}
                  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setCycleLength(int time) {
  cycle_length_ = time;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int BatchReactor::cycleLength() { 
  return cycle_length_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setCoreLoading(double size) {
  core_loading_ = size;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double BatchReactor::coreLoading() {
  return core_loading_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setNBatches(int n) {
  batches_per_core_ = n;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int BatchReactor::nBatches() {
  return batches_per_core_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double BatchReactor::batchLoading() {
  return core_loading_ / batches_per_core_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setInCommodity(std::string name) {
  in_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string BatchReactor::inCommodity() {
  return in_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setInRecipe(std::string name) {
  in_recipe_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string BatchReactor::inRecipe() {
  return in_recipe_;
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setOutCommodity(std::string name) {
  out_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string BatchReactor::outCommodity() {
  return out_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setOutRecipe(std::string name) {
  out_recipe_ = name;
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string BatchReactor::outRecipe() {
  return out_recipe_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Phase BatchReactor::phase() {
  return phase_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool BatchReactor::checkDecommissionCondition() {
  bool empty = (preCore_.empty() && inCore_.empty() && 
                postCore_.empty());
  // if (!empty) {
  //   string msg = "Can't delete a BatchReactor with material still in its inventory.";
  //   throw CycBatchReactorDestructException(msg);
  // }
  return empty;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::setPhase(Phase p) {
  phase_ = p;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::resetCycleTimer() {
  cycle_timer_ = 1;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool BatchReactor::cycleComplete() {
  return (cycle_timer_ >= cycle_length_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
bool BatchReactor::coreFilled() {
  return (abs(inCore_.quantity() - inCore_.capacity()) < EPS_KG);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::makeRequest(double amt) {
  interactWithMarket(inCommodity(),amt,REQUEST);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::makeOffers() {
  if (!postCore_.empty()) {
    interactWithMarket(outCommodity(),postCore_.quantity(),OFFER);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::interactWithMarket(std::string commod, double amt, TransType type) {
  LOG(LEV_INFO4, "BReact") << " making requests {";  
  // get the market
  MarketModel* market = MarketModel::marketForCommod(commod);
  Communicator* recipient = dynamic_cast<Communicator*>(market);
  // set the price
  double commod_price = 0;
  // request a generic resource
  gen_rsrc_ptr trade_res = gen_rsrc_ptr(new GenericResource(commod, "kg", amt));
  // build the transaction and message
  Transaction trans(this, type);
  trans.setCommod(commod);
  trans.setMinFrac(1.0);
  trans.setPrice(commod_price);
  trans.setResource(trade_res);
  // log the event
  string text;
  if (type == OFFER) {
    text = " has offered ";
  }
  else {
    text = " has requested ";
  }
  LOG(LEV_INFO5, "BReact") << name() << text << amt
                           << " kg of " << commod << ".";
  // send the message
  sendMessage(recipient, trans);
  LOG(LEV_INFO4, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::handleOrders() {
  while(!ordersWaiting_.empty()){
    msg_ptr order = ordersWaiting_.front();
    order->trans().approveTransfer();
    ordersWaiting_.pop_front();
  };
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::moveFuel(MatBuff& fromBuff, MatBuff& toBuff, double amt) {
  vector<mat_rsrc_ptr> to_move = fromBuff.popQty(amt);
  for (int i = 0; i < to_move.size(); i++) {
    mat_rsrc_ptr newMat = mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(out_recipe_)));
    newMat->setQuantity(to_move.at(i)->quantity());
    toBuff.pushOne(newMat);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::moveFuel(MatBuff& fromBuff, MatBuff& toBuff) {
  if (!fromBuff.empty()) // @MJGFlag this check shouldn't have to be made
    moveFuel(fromBuff,toBuff,fromBuff.quantity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::loadCore() {
  moveFuel(preCore_,inCore_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::offloadBatch() {
  moveFuel(inCore_,postCore_,batchLoading());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BatchReactor::offloadCore() {
  moveFuel(inCore_,postCore_);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructBatchReactor() {
  return new BatchReactor();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructBatchReactor(Model* model) {
      delete model;
}
