// sink_facility.cc
// Implements the SinkFacility class
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "sink_facility.h"

#include "context.h"
#include "logger.h"
#include "generic_resource.h"
#include "error.h"
#include "cyc_limits.h"
#include "market_model.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::SinkFacility(cyclus::Context* ctx)
    : cyclus::FacilityModel(ctx),
      commod_price_(0),
      capacity_(std::numeric_limits<double>::max()) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::~SinkFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SinkFacility::schema() {
  return
    "  <element name =\"input\">          \n"
    "    <element name = \"commodities\"> \n"
    "      <oneOrMore>                    \n"
    "        <ref name=\"incommodity\"/>  \n"
    "      </oneOrMore>                   \n"
    "    </element>                       \n"
    "     <optional>                      \n"
    "      <ref name=\"input_capacity\"/> \n"
    "    </optional>                      \n"
    "    <optional>                       \n"
    "      <ref name=\"inventorysize\"/>  \n"
    "    </optional>                      \n"
    "  </element>                         \n";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  using std::numeric_limits;
  using boost::lexical_cast;
  cyclus::QueryEngine* input = qe->QueryElement("input");

  cyclus::QueryEngine* commodities = input->QueryElement("commodities");
  string query = "incommodity";
  int nCommodities = commodities->NElementsMatchingQuery(query);
  for (int i = 0; i < nCommodities; i++) {
    AddCommodity(commodities->GetElementContent(query, i));
  }
  
  double capacity =
      cyclus::GetOptionalQuery<double>(input,
                                       "input_capacity",
                                       numeric_limits<double>::max());
  SetCapacity(capacity);

  double size =
      cyclus::GetOptionalQuery<double>(input,
                                       "inventorysize",
                                       numeric_limits<double>::max());
  SetMaxInventorySize(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SinkFacility::str() {
  using std::string;
  using std::vector;
  std::stringstream ss;
  ss << cyclus::FacilityModel::str();

  string msg = "";
  msg += "accepts commodities ";
  for (vector<string>::iterator commod = in_commods_.begin();
       commod != in_commods_.end();
       commod++) {
    msg += (commod == in_commods_.begin() ? "{" : ", ");
    msg += (*commod);
  }
  msg += "} until its inventory is full at ";
  ss << msg << inventory_.capacity() << " kg.";
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SinkFacility::Clone() {
  SinkFacility* m = new SinkFacility(*this);
  m->InitFrom(this);

  m->SetCapacity(capacity());
  m->SetMaxInventorySize(MaxInventorySize());
  m->in_commods_ = InputCommodities();

  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::HandleTick(int time) {
  using std::string;
  using std::vector;
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is ticking {";

  double requestAmt = getRequestAmt();
  CLOG(cyclus::LEV_DEBUG3) << "SinkFacility " << name() << " on the tick has "
                           << "a request amount of: " << requestAmt;
  double minAmt = 0;


  if (requestAmt > cyclus::eps()) {

    // for each potential commodity, make a request
    for (vector<string>::iterator commod = in_commods_.begin();
         commod != in_commods_.end();
         commod++) {
      LOG(cyclus::LEV_INFO4, "SnkFac") << " requests " << requestAmt << " kg of " <<
                                       *commod << ".";

      cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(*commod);
      cyclus::Communicator* recipient = dynamic_cast<cyclus::Communicator*>(market);

      // create a generic resource
      cyclus::GenericResource::Ptr request_res =
          cyclus::GenericResource::Create(this,
                                          requestAmt,
                                          "kg",
                                          *commod);

      // build the transaction and message
      cyclus::Transaction trans(this, cyclus::REQUEST);
      trans.SetCommod(*commod);
      trans.SetMinFrac(minAmt / requestAmt);
      trans.SetPrice(commod_price_);
      trans.SetResource(request_res);

      cyclus::Message::Ptr request(new cyclus::Message(this, recipient, trans));
      request->SendOn();

    }
  }
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SnkFac") << "SinkFacility " << this->id()
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
void SinkFacility::SetMaxInventorySize(double size) {
  inventory_.set_capacity(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SinkFacility::MaxInventorySize() {
  return inventory_.capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SinkFacility::InventorySize() {
  return inventory_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::vector<std::string> SinkFacility::InputCommodities() {
  return in_commods_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::AddResource(cyclus::Transaction trans,
                               std::vector<cyclus::Resource::Ptr> manifest) {
  inventory_.PushAll(manifest);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SinkFacility::getRequestAmt() {
  // The sink facility should ask for as much stuff as it can reasonably receive.
  double requestAmt;
  // get current capacity
  double space = inventory_.space();

  if (space <= 0) {
    requestAmt = 0;
  } else if (space < capacity_) {
    requestAmt = space / in_commods_.size();
  } else if (space >= capacity_) {
    requestAmt = capacity_ / in_commods_.size();
  }
  return requestAmt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructSinkFacility(cyclus::Context* ctx) {
  return new SinkFacility(ctx);
}
} // namespace cycamore
