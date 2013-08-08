// storage_facility.cc
// Implements the StorageFacility class
#include "storage_facility.h"

#include "timer.h"

#include <sstream>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

/**
  cyclus::TICK
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
  offer_price_(0.0) {
  out_buffer_.SetCapacity(kBuffInfinity);
  in_buffer_.SetCapacity(kBuffInfinity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StorageFacility::~StorageFacility() {
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StorageFacility::str() {
  std::stringstream ss("");
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StorageFacility::ReceiveMessage(cyclus::Message::Ptr msg) {
  // is this a message from on high?
  if (msg->trans().supplier() == this) {
    // file the order
    ordersWaiting_.push_front(msg);
  } else {
    throw cyclus::CycException("StorageFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Transaction StorageFacility::BuildTransaction() {
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = out_buffer_.quantity();

  cyclus::GenericResource::Ptr offer_res =
    cyclus::GenericResource::Ptr(new cyclus::GenericResource(out_commod_, "kg",
                                                             offer_amt));

  cyclus::Transaction trans(this, cyclus::OFFER);
  trans.SetCommod(out_commod_);
  trans.SetMinFrac(min_amt / offer_amt);
  trans.SetPrice(offer_price_);
  trans.SetResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StorageFacility::SendOffer(cyclus::Transaction trans) {
  cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  cyclus::Message::Ptr msg(new cyclus::Message(this, recipient, trans));
  msg->SendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StorageFacility::HandleTick(int time) {
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructStorageFacility() {
  return new StorageFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructStorageFacility(cyclus::Model* model) {
  delete model;
}
