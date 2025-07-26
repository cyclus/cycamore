#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include "toolkit/mat_query.h"

#include "conversion.h"

using cyclus::RequestPortfolio;
using cyclus::BidPortfolio;
using cyclus::CommodMap;
using cyclus::Request;
using cyclus::Trade;
using cyclus::Material;
using cyclus::CapacityConstraint;
using cyclus::toolkit::ResBuf;

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Conversion::Conversion(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {

      // Make our Resource Buffers bulk buffers
      input = ResBuf<Material>(true);
      output = ResBuf<Material>(true);
      waste = ResBuf<Material>(true);
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Conversion::~Conversion() {}

#pragma cyclus def schema cycamore::Conversion
#pragma cyclus def annotations cycamore::Conversion
#pragma cyclus def infiletodb cycamore::Conversion
#pragma cyclus def snapshot cycamore::Conversion
#pragma cyclus def snapshotinv cycamore::Conversion
#pragma cyclus def initinv cycamore::Conversion
#pragma cyclus def clone cycamore::Conversion
#pragma cyclus def initfromdb cycamore::Conversion
#pragma cyclus def initfromcopy cycamore::Conversion

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::EnterNotify() {
  cyclus::Facility::EnterNotify();
  InitializePosition();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Conversion::str() {
  using std::string;
  using std::vector;
  std::stringstream ss;
  ss << cyclus::Facility::str();

  string msg = "";
  msg += "accepts commodities ";
  for (vector<string>::iterator commod = incommods.begin();
       commod != incommods.end();
       commod++) {
    msg += (commod == incommods.begin() ? "{" : ", ");
    msg += (*commod);
  }
  msg += "} until its input is full at ";
  msg += std::to_string(input.capacity());
  msg += " kg.";

  msg += "} and converts into commodity ";
  ss << msg << outcommod;
  return "" + ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::Tick() {
  ConvertUranium();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::Tock() {
  
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Conversion::CalculateNeededFeedstock() {
  return input.capacity() - input.quantity();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::ConvertUranium() {
  if (input.quantity() <= 0) return;
  else if (input.quantity() < throughput) {
    output.Push(input.Pop(input.quantity()));
  } else {
    output.Push(input.Pop(throughput));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<RequestPortfolio<Material>::Ptr> Conversion::GetMatlRequests() {
  std::set<RequestPortfolio<Material>::Ptr> ports;

  // Check if we need material
  double needed = CalculateNeededFeedstock();
  if (needed <= 0) return ports;

  // Create request portfolio
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

  // Create material request
  Material::Ptr dummy = Material::CreateUntracked(needed,
                                                 context()->GetRecipe(inrecipe_name));

  // Add request for all commodities using default preference
  for (std::vector<std::string>::iterator it = incommods.begin();
       it != incommods.end(); ++it) {
    Request<Material>* req = port->AddRequest(dummy, this, *it);
  }

  // Add capacity constraint to ensure we never get more feed than needed
  CapacityConstraint<Material> cc(needed);
  port->AddConstraint(cc);

  ports.insert(port);
  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<BidPortfolio<Material>::Ptr> Conversion::GetMatlBids(
  CommodMap<Material>::type& commod_requests) {
  std::set<BidPortfolio<Material>::Ptr> ports;

  // Check if we have material to offer
  if (output.quantity() <= 0) return ports;

  // Create bid portfolio
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  // Respond to requests for our commodity
  std::vector<Request<Material>*>& requests = commod_requests[outcommod];
  for (std::vector<Request<Material>*>::iterator it = requests.begin();
      it != requests.end(); ++it) {

    double available = output.quantity();
    double requested = (*it)->target()->quantity();
    double offer_qty = std::min(available, requested);

    // This part may be wrong. Not sure if we really should pop here.
    if (offer_qty > 0) {
      Material::Ptr offer = Material::CreateUntracked(offer_qty, output.Peek()->comp());
      port->AddBid(*it, offer, this);  // Note: *it, not **it
    }
  }

  // Add capacity constraint so we never give out more than we have
  CapacityConstraint<Material> cc(output.quantity());
  port->AddConstraint(cc);

  ports.insert(port);
  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::AcceptMatlTrades(
  const std::vector<std::pair<Trade<Material>, Material::Ptr>>& responses) {

  for (std::vector<std::pair<Trade<Material>, Material::Ptr>>::const_iterator it =
      responses.begin(); it != responses.end(); ++it) {

    Material::Ptr mat = it->second;
    cyclus::toolkit::MatQuery mq(mat);

    // Check if there's uranium in the material
    double u_mass = mq.mass(922350000) + mq.mass(922380000);

    // If there's uranium, add it to the input buffer, otherwise add it to waste
    if (u_mass > 0) {
      input.Push(mat);
    } else {
      waste.Push(mat);
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Conversion::GetMatlTrades(
  const std::vector<Trade<Material>>& trades,
  std::vector<std::pair<Trade<Material>, Material::Ptr>>& responses) {

  for (std::vector<Trade<Material>>::const_iterator it = trades.begin();
      it != trades.end(); ++it) {

    double requested_qty = it->amt;
    Material::Ptr response = output.Pop(requested_qty);

    responses.push_back(std::make_pair(*it, response));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructConversion(cyclus::Context* ctx) {
  return new Conversion(ctx);
}

}  // namespace cycamore

