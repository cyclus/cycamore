// sink_facility.cc
// Implements the SinkFacility class
#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "capacity_constraint.h"
#include "context.h"
#include "cyc_limits.h"
#include "error.h"
#include "logger.h"

#include "sink_facility.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::SinkFacility(cyclus::Context* ctx)
  : cyclus::FacilityModel(ctx),
    cyclus::Model(ctx),
    commod_price_(0),
    capacity_(std::numeric_limits<double>::max()) {
  SetMaxInventorySize(std::numeric_limits<double>::max());
}

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
void SinkFacility::InitFrom(cyclus::QueryEngine* qe) {
  cyclus::FacilityModel::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());
  
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

  double cap = cyclus::GetOptionalQuery<double>(input,
                                                "input_capacity",
                                                numeric_limits<double>::max());
  capacity(cap);

  double size = cyclus::GetOptionalQuery<double>(input,
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
  SinkFacility* m = new SinkFacility(context());
  m->InitFrom(this);
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::InitFrom(SinkFacility* m) {
  FacilityModel::InitFrom(m);
  
  capacity(m->capacity());
  SetMaxInventorySize(m->MaxInventorySize());
  capacity_ = m->capacity_;
  in_commods_ = m->in_commods_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
SinkFacility::GetMatlRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  
  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = RequestAmt();
  Material::Ptr mat = Material::CreateBlank(amt);

  if (amt > cyclus::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);
    
    std::vector<std::string>::const_iterator it;
    for (it = in_commods_.begin(); it != in_commods_.end(); ++it) {
      port->AddRequest(mat, this, *it);
    }
    
    ports.insert(port);
  } // if amt > eps

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::set<cyclus::RequestPortfolio<cyclus::GenericResource>::Ptr>
SinkFacility::GetGenRsrcRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::GenericResource;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  
  std::set<RequestPortfolio<GenericResource>::Ptr> ports;
  RequestPortfolio<GenericResource>::Ptr
      port(new RequestPortfolio<GenericResource>());
  double amt = RequestAmt();

  if (amt > cyclus::eps()) {
    CapacityConstraint<GenericResource> cc(amt);
    port->AddConstraint(cc);
    
    std::vector<std::string>::const_iterator it;
    for (it = in_commods_.begin(); it != in_commods_.end(); ++it) {
      std::string quality = ""; // not clear what this should be..
      std::string units = ""; // not clear what this should be..
      GenericResource::Ptr rsrc = GenericResource::CreateUntracked(amt,
                                                                   quality,
                                                                   units);
      port->AddRequest(rsrc, this, *it);
    }
    
    ports.insert(port);
  } // if amt > eps
  
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory_.Push(it->second);    
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::AcceptGenRsrcTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::GenericResource>,
                                 cyclus::GenericResource::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::GenericResource>,
                         cyclus::GenericResource::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory_.Push(it->second);    
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::Tick(int time) {
  using std::string;
  using std::vector;
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is ticking {";

  double requestAmt = RequestAmt();
  // inform the simulation about what the sink facility will be requesting
  if (requestAmt > cyclus::eps()) {
    for (vector<string>::iterator commod = in_commods_.begin();
         commod != in_commods_.end();
         commod++) {
      LOG(cyclus::LEV_INFO4, "SnkFac") << " will request " << requestAmt
                                       << " kg of " << *commod << ".";
    }
  }
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::Tock(int time) {
  LOG(cyclus::LEV_INFO3, "SnkFac") << FacName() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SnkFac") << "SinkFacility " << this->id()
                                   << " is holding " << inventory_.quantity()
                                   << " units of material at the close of month "
                                   << time << ".";
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructSinkFacility(cyclus::Context* ctx) {
  return new SinkFacility(ctx);
}
} // namespace cycamore
