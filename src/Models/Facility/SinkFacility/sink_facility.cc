// sink_facility.cc
// Implements the SinkFacility class
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "sink_facility.h"

#include "logger.h"
#include "generic_resource.h"
#include "error.h"
#include "cyc_limits.h"
#include "market_model.h"

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::SinkFacility() :
  commod_price_(0),
  capacity_(numeric_limits<double>::max()) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::~SinkFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  cyclus::QueryEngine* input = qe->QueryElement("input");

  cyclus::QueryEngine* commodities = input->QueryElement("commodities");
  string query = "incommodity";
  int nCommodities = commodities->NElementsMatchingQuery(query);
  for (int i = 0; i < nCommodities; i++) {
    AddCommodity(commodities->GetElementContent(query,i));
  }

  string data;
  try {
    data = input->GetElementContent("input_capacity");
    SetCapacity(lexical_cast<double>(data));
  } catch (cyclus::Error e) {
    SetCapacity(numeric_limits<double>::max());
  }

  try {
    data = input->GetElementContent("inventorysize");
    setMaxInventorySize(lexical_cast<double>(data));
  } catch (cyclus::Error e) {
    setMaxInventorySize(numeric_limits<double>::max());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SinkFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str();

  string msg = "";
  msg += "accepts commodities ";
  for (vector<string>::iterator commod=in_commods_.begin();
       commod != in_commods_.end();
       commod++) {
    msg += (commod == in_commods_.begin() ? "{" : ", " );
    msg += (*commod);
  }
  msg += "} until its inventory is full at ";
  ss << msg << inventory_.capacity() << " kg.";
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::CloneModuleMembersFrom(cyclus::FacilityModel* sourceModel) {
  SinkFacility* source = dynamic_cast<SinkFacility*>(sourceModel);
  SetCapacity(source->capacity());
  setMaxInventorySize(source->maxInventorySize());
  in_commods_ = source->InputCommodities();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::HandleTick(int time){
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is ticking {";

  double requestAmt = getRequestAmt();
  CLOG(cyclus::LEV_DEBUG3) << "SinkFacility " << name() << " on the tick has "
                   << "a request amount of: " << requestAmt;
  double minAmt = 0;


  if (requestAmt>cyclus::eps()){

    // for each potential commodity, make a request
    for (vector<string>::iterator commod = in_commods_.begin();
        commod != in_commods_.end();
        commod++) {
      LOG(cyclus::LEV_INFO4, "SnkFac") << " requests "<< requestAmt << " kg of " << *commod << ".";

      cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(*commod);
      cyclus::Communicator* recipient = dynamic_cast<cyclus::Communicator*>(market);

      // create a generic resource
      cyclus::GenericResource::Ptr request_res = cyclus::GenericResource::Ptr(new cyclus::GenericResource("kg",*commod,requestAmt));

      // build the transaction and message
      cyclus::Transaction trans(this, cyclus::REQUEST);
      trans.SetCommod(*commod);
      trans.SetMinFrac(minAmt/requestAmt);
      trans.SetPrice(commod_price_);
      trans.SetResource(request_res);

      cyclus::Message::Ptr request(new cyclus::Message(this, recipient, trans));
      request->SendOn();

    }
  }
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::HandleTock(int time){
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SnkFac") << "SinkFacility " << this->ID()
                  << " is holding " << inventory_.quantity()
                  << " units of material at the close of month " << time
                  << ".";
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::AddCommodity(std::string name) {
  in_commods_.push_back(name);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::SetCapacity(double capacity) {
  capacity_ = capacity;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SinkFacility::capacity() {
  return capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::setMaxInventorySize(double size) {
  inventory_.SetCapacity(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SinkFacility::maxInventorySize() {
  return inventory_.capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SinkFacility::inventorySize() {
  return inventory_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::vector<std::string> SinkFacility::InputCommodities() {
  return in_commods_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::AddResource(cyclus::Transaction trans, std::vector<cyclus::Resource::Ptr> manifest) {
  inventory_.PushAll(cyclus::MatBuff::ToMat(manifest));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SinkFacility::getRequestAmt(){
  // The sink facility should ask for as much stuff as it can reasonably receive.
  double requestAmt;
  // get current capacity
  double space = inventory_.space();

  if (space <= 0 ){
    requestAmt=0;
  } else if (space < capacity_){
    requestAmt = space/in_commods_.size();
  } else if (space >= capacity_){
    requestAmt = capacity_/in_commods_.size();
  }
  return requestAmt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructSinkFacility() {
  return new SinkFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructSinkFacility(cyclus::Model* model) {
      delete model;
}
