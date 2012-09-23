// SinkFacility.cpp
// Implements the SinkFacility class
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "SinkFacility.h"

#include "Logger.h"
#include "GenericResource.h"
#include "CycException.h"
#include "MarketModel.h"

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
SinkFacility::SinkFacility() :
  commod_price_(0), 
  capacity_(numeric_limits<double>::max()) 
{
  in_commods_ = vector<string>();
  inventory_ = MatBuff();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
SinkFacility::~SinkFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::initModuleMembers(QueryEngine* qe) {
  QueryEngine* input = qe->queryElement("input");

  QueryEngine* commodities = input->queryElement("commodities");
  string query = "incommodity";
  int nCommodities = commodities->nElementsMatchingQuery(query);
  for (int i = 0; i < nCommodities; i++) {
    addCommodity(commodities->getElementContent(query,i));
  }

  string data;
  data = input->getElementContent("input_capacity"); 
  setCapacity(lexical_cast<double>(data));  
  try {
    data = input->getElementContent("inventorysize"); 
    setMaxInventorySize(lexical_cast<double>(data));
  } catch (CycNullQueryException e) {
    setMaxInventorySize(numeric_limits<double>::max());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string SinkFacility::str() {
  std::stringstream ss;
  ss << FacilityModel::str();

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
void SinkFacility::cloneModuleMembersFrom(FacilityModel* sourceModel) {
  SinkFacility* source = dynamic_cast<SinkFacility*>(sourceModel);
  setCapacity(source->capacity());
  setMaxInventorySize(source->maxInventorySize());
  in_commods_ = source->inputCommodities();
  CLOG(LEV_DEBUG3) << "SinkFacility cloned: " << str();
  CLOG(LEV_DEBUG3) << "               From: " << sourceModel->str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::handleTick(int time){
  LOG(LEV_INFO3, "SnkFac") << facName() << " is ticking {";

  double requestAmt = getRequestAmt(); 
  CLOG(LEV_DEBUG3) << "SinkFacility " << name() << " on the tick has "
                   << "a request amount of: " << requestAmt;
  double minAmt = 0;


  if (requestAmt>EPS_KG){

    // for each potential commodity, make a request
    for (vector<string>::iterator commod = in_commods_.begin();
        commod != in_commods_.end();
        commod++) {
      LOG(LEV_INFO4, "SnkFac") << " requests "<< requestAmt << " kg of " << *commod << ".";

      MarketModel* market = MarketModel::marketForCommod(*commod);
      Communicator* recipient = dynamic_cast<Communicator*>(market);

      // create a generic resource
      gen_rsrc_ptr request_res = gen_rsrc_ptr(new GenericResource((*commod), "kg", requestAmt));

      // build the transaction and message
      Transaction trans(this, REQUEST);
      trans.setCommod(*commod);
      trans.setMinFrac(minAmt/requestAmt);
      trans.setPrice(commod_price_);
      trans.setResource(request_res);

      msg_ptr request(new Message(this, recipient, trans)); 
      request->sendOn();

    }
  }
  LOG(LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::handleTock(int time){
  LOG(LEV_INFO3, "SnkFac") << facName() << " is tocking {";

  // On the tock, the sink facility doesn't really do much. 
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(LEV_INFO4, "SnkFac") << "SinkFacility " << this->ID()
                  << " is holding " << inventory_.quantity()
                  << " units of material at the close of month " << time
                  << ".";
  LOG(LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::addCommodity(std::string name) {
  in_commods_.push_back(name);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::setCapacity(double capacity) {
  capacity_ = capacity;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
double SinkFacility::capacity() {
  return capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::setMaxInventorySize(double size) {
  inventory_.setCapacity(size);
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
std::vector<std::string> SinkFacility::inputCommodities() {
  return in_commods_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::addResource(Transaction trans, std::vector<rsrc_ptr> manifest) {
  inventory_.pushAll(MatBuff::toMat(manifest));
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
extern "C" Model* constructSinkFacility() {
  return new SinkFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructSinkFacility(Model* model) {
      delete model;
}
