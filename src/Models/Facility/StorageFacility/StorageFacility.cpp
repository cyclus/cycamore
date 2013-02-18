// StorageFacility.cpp
// Implements the StorageFacility class
#include "StorageFacility.h"

#include "Timer.h"

#include <sstream>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

/**
  TICK
  send a request for your capacity minus your stocks.
  offer materials that have exceeded their residence times
  offer them
 
  TOCK
  send appropriate materials to fill ordersWaiting.
 
  RECIEVE MATERIAL
  put it in stocks
  add it to the deque of release times
 
  SEND MATERIAL
  pull it from inventory, fill the transaction
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
StorageFacility::StorageFacility() :
  out_commod_(""),
  in_commod_(""),
  residence_time_(0),
  offer_price_(0.0)
{
  out_buffer_.setCapacity(kBuffInfinity);
  in_buffer_.setCapacity(kBuffInfinity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
StorageFacility::~StorageFacility()
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string StorageFacility::str() 
{ 
  std::stringstream ss("");
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void StorageFacility::receiveMessage(msg_ptr msg)
{
  // is this a message from on high? 
  if(msg->trans().supplier()==this){
    // file the order
    ordersWaiting_.push_front(msg);
  }
  else {
    throw CycException("StorageFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Transaction StorageFacility::buildTransaction() {
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = out_buffer_.quantity();

  gen_rsrc_ptr offer_res = 
    gen_rsrc_ptr(new GenericResource(out_commod_,"kg",offer_amt));

  Transaction trans(this, OFFER);
  trans.setCommod(out_commod_);
  trans.setMinFrac(min_amt/offer_amt);
  trans.setPrice(offer_price_);
  trans.setResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void StorageFacility::sendOffer(Transaction trans) {
  MarketModel* market = MarketModel::marketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  msg_ptr msg(new Message(this, recipient, trans)); 
  msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void StorageFacility::handleTick(int time)
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" Model* constructStorageFacility() {
  return new StorageFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructStorageFacility(Model* model) {
      delete model;
}
