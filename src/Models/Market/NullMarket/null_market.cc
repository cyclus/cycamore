// null_market.cc
// Implements the NullMarket class
#include <iostream>

#include "null_market.h"

#include "cyc_limits.h"
#include "resource.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullMarket::NullMarket() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullMarket::~NullMarket() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void NullMarket::ReceiveMessage(cyclus::Message::Ptr msg) {
  messages_.insert(msg);

  if (msg->trans().IsOffer()){
    offers_.insert(IndexedMsg(msg->trans().resource()->quantity(),msg));
  }
  else if (!msg->trans().IsOffer()){
    requests_.insert(IndexedMsg(msg->trans().resource()->quantity(),msg));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void NullMarket::reject_request(SortedMsgList::iterator request)
{
  // delete the tentative orders
  while ( orders_.size() > firmOrders_) {
    orders_.pop_back();
  }

  // put all matched offers_ back in the sorted list
  while (matchedOffers_.size() > 0) {
    cyclus::Message::Ptr msg = *(matchedOffers_.begin());
    offers_.insert(IndexedMsg(msg->trans().resource()->quantity(),msg));
    matchedOffers_.erase(msg);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void NullMarket::process_request()
{
  // update pointer to firm orders
  firmOrders_ = orders_.size();

  while (matchedOffers_.size() > 0)
  {
    cyclus::Message::Ptr msg = *(matchedOffers_.begin());
    messages_.erase(msg);
    matchedOffers_.erase(msg);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool NullMarket::match_request(SortedMsgList::iterator request)
{
  SortedMsgList::iterator offer;
  double requestAmt, offerAmt, toRet;
  cyclus::Message::Ptr offerMsg;
  cyclus::Message::Ptr requestMsg;

  requestAmt = request->first;
  requestMsg = request->second;

  // if this request is not yet satisfied &&
  // there are more offers_ left
  while ( requestAmt > 0 && offers_.size() > 0)
  {
    // get a new offer
    offer = offers_.end();
    offer--;
    offerAmt = offer->first;
    offerMsg = offer->second;

    // pop off this offer
    offers_.erase(offer);

    if (requestMsg->trans().resource()->CheckQuality(offerMsg->trans().resource())){

      LOG(cyclus::LEV_DEBUG1,"NulMkt") << "Comparing " << requestAmt << " >= "
                               << offerAmt
                               << ": " << (requestAmt>=offerAmt);

      if (requestAmt >= offerAmt) {
        // put a new message in the order stack
        // it goes down to supplier
        offerMsg->trans().MatchWith(requestMsg->trans());

        // Queue an order
        matchedOffers_.insert(offerMsg);
        // Zero out the boolean.
        // At least some of the request will be met.
        toRet = 0;

        orders_.push_back(offerMsg);

        LOG(cyclus::LEV_DEBUG2, "none!")
	  << "null_market.has resolved a transaction "
	  << " which is a match from "
          << offerMsg->trans().supplier()->ID()
          << " to "
          << offerMsg->trans().requester()->ID()
          << " for the amount:  "
          << offerMsg->trans().resource()->quantity();

        requestAmt -= offerAmt;
      }
      else {
        // split offer

        // queue a new order
        cyclus::Message::Ptr maybe_offer = offerMsg->clone();
        maybe_offer->trans().resource()->SetQuantity(requestAmt);
        maybe_offer->trans().MatchWith(requestMsg->trans());
        maybe_offer->trans().SetResource(requestMsg->trans().resource());
        matchedOffers_.insert(offerMsg);

        orders_.push_back(maybe_offer);

        LOG(cyclus::LEV_DEBUG2, "none!") << "null_market.has resolved a match from "
                                 << maybe_offer->trans().supplier()->ID()
                                 << " to "
                                 << maybe_offer->trans().requester()->ID()
                                 << " for " << maybe_offer->trans().resource()->quantity()
                                 << " of ";
        maybe_offer->trans().resource()->Print();

        // reduce the offer amount
        offerAmt -= requestAmt;

        // if the residual is above threshold,
        // make a new offer with reduced amount

        if(offerAmt > cyclus::eps()){
          cyclus::Message::Ptr new_offer = offerMsg->clone();
          new_offer->trans().resource()->SetQuantity(offerAmt);
          ReceiveMessage(new_offer);
        }

        // zero out request
        requestAmt = 0;

        // Zero out the boolean.
        // All of the request will be met.
        toRet = 0;
      }
    }
  }

  // if any of the request has been met, return true.
  return (0 == toRet);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void NullMarket::Resolve()
{
  SortedMsgList::iterator request;

  firmOrders_ = 0;

  /// while requests_ remain and there is at least one offer left
  while (requests_.size() > 0)
  {
    request = requests_.end();
    request--;

    if(match_request(request)) {
      process_request();
    }
    else {
      LOG(cyclus::LEV_DEBUG2, "none!") << "The request from Requester "<< (*request).second->trans().requester()->ID()
          << " for the amount " << (*request).first
          << " rejected. ";
      reject_request(request);
    }
    // remove this request
    messages_.erase((*request).second);
    requests_.erase(request);
  }

  for (int i = 0; i < orders_.size(); i++) {
    cyclus::Message::Ptr msg = orders_.at(i);
    msg->SetDir(cyclus::DOWN_MSG);
    msg->SendOn();
  }
  orders_.clear();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructNullMarket() {
  return new NullMarket();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructNullMarket(cyclus::Model* model) {
      delete model;
}
