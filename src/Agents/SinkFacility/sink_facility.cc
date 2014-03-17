// sink_facility.cc
// Implements the SinkFacility class
#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "sink_facility.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SinkFacility::SinkFacility(cyc::Context* ctx)
  : cyc::Facility(ctx),
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

void SinkFacility::InfileToDb(cyc::InfileTree* qe, cyc::DbInit di) {
  cyc::Facility::InfileToDb(qe, di);
  qe = qe->QueryElement("agent/" + agent_impl());
  
  using std::numeric_limits;
  cyc::InfileTree* input = qe->QueryElement("input");

  cyc::InfileTree* commodities = input->QueryElement("commodities");
  int n = commodities->NElementsMatchingQuery("incommodity");
  for (int i = 0; i < n; i++) {
    di.NewDatum("InCommods")
      ->AddVal("commod", commodities->GetString("incommodity", i))
      ->Record();
  }

  double cap = cyc::GetOptionalQuery<double>(input,
                                             "input_capacity",
                                             numeric_limits<double>::max());
  double size = cyc::GetOptionalQuery<double>(input,
                                              "inventorysize",
                                              numeric_limits<double>::max());
  di.NewDatum("Info")
    ->AddVal("capacity", cap)
    ->AddVal("commod_price", 0)
    ->AddVal("max_inv_size", size)
    ->Record();
}

void SinkFacility::InitFrom(cyc::QueryableBackend* b) {
  cyc::Facility::InitFrom(b);

  cyc::QueryResult qr = b->Query("Info", NULL);
  capacity_ = qr.GetVal<double>("capacity");
  commod_price_ = qr.GetVal<double>("commod_price");
  inventory_.set_capacity(qr.GetVal<double>("max_inv_size"));
  qr = b->Query("InCommods", NULL);
  for (int i = 0; i < qr.rows.size(); ++i) {
    in_commods_.push_back(qr.GetVal<std::string>("commod", i));
  }
}

void SinkFacility::Snapshot(cyc::DbInit di) {
  cyc::Facility::Snapshot(di);
  di.NewDatum("Info")
    ->AddVal("capacity", capacity_)
    ->AddVal("commod_price", commod_price_)
    ->AddVal("max_inv_size", inventory_.capacity())
    ->Record();
  for (int i = 0; i < in_commods_.size(); ++i) {
    di.NewDatum("InCommods")
      ->AddVal("commod", in_commods_[i])
      ->Record();
  }
}

void SinkFacility::InitInv(cyc::Inventories& inv) {
  inventory_.PushAll(inv["inventory"]);
}

cyc::Inventories SinkFacility::SnapshotInv() {
  cyc::Inventories invs;
  invs["inventory"] = inventory_.PopN(inventory_.count());
  return invs;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SinkFacility::str() {
  using std::string;
  using std::vector;
  std::stringstream ss;
  ss << cyc::Facility::str();

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
cyc::Agent* SinkFacility::Clone() {
  SinkFacility* m = new SinkFacility(context());
  m->InitFrom(this);
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::InitFrom(SinkFacility* m) {
  Facility::InitFrom(m);
  capacity(m->capacity());
  SetMaxInventorySize(m->MaxInventorySize());
  capacity_ = m->capacity_;
  in_commods_ = m->in_commods_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::set<cyc::RequestPortfolio<cyc::Material>::Ptr>
SinkFacility::GetMatlRequests() {
  using cyc::CapacityConstraint;
  using cyc::Material;
  using cyc::RequestPortfolio;
  using cyc::Request;
  
  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = RequestAmt();
  Material::Ptr mat = cyc::NewBlankMaterial(amt);

  if (amt > cyc::eps()) {
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
std::set<cyc::RequestPortfolio<cyc::Product>::Ptr>
SinkFacility::GetGenRsrcRequests() {
  using cyc::CapacityConstraint;
  using cyc::Product;
  using cyc::RequestPortfolio;
  using cyc::Request;
  
  std::set<RequestPortfolio<Product>::Ptr> ports;
  RequestPortfolio<Product>::Ptr
      port(new RequestPortfolio<Product>());
  double amt = RequestAmt();

  if (amt > cyc::eps()) {
    CapacityConstraint<Product> cc(amt);
    port->AddConstraint(cc);
    
    std::vector<std::string>::const_iterator it;
    for (it = in_commods_.begin(); it != in_commods_.end(); ++it) {
      std::string quality = ""; // not clear what this should be..
      Product::Ptr rsrc = Product::CreateUntracked(amt,
                                                                   quality);
      port->AddRequest(rsrc, this, *it);
    }
    
    ports.insert(port);
  } // if amt > eps
  
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::AcceptMatlTrades(
    const std::vector< std::pair<cyc::Trade<cyc::Material>,
                                 cyc::Material::Ptr> >& responses) {
  std::vector< std::pair<cyc::Trade<cyc::Material>,
                         cyc::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory_.Push(it->second);    
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::AcceptGenRsrcTrades(
    const std::vector< std::pair<cyc::Trade<cyc::Product>,
                                 cyc::Product::Ptr> >& responses) {
  std::vector< std::pair<cyc::Trade<cyc::Product>,
                         cyc::Product::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory_.Push(it->second);    
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void SinkFacility::Tick(int time) {
  using std::string;
  using std::vector;
  LOG(cyc::LEV_INFO3, "SnkFac") << prototype() << " is ticking {";

  double requestAmt = RequestAmt();
  // inform the simulation about what the sink facility will be requesting
  if (requestAmt > cyc::eps()) {
    for (vector<string>::iterator commod = in_commods_.begin();
         commod != in_commods_.end();
         commod++) {
      LOG(cyc::LEV_INFO4, "SnkFac") << " will request " << requestAmt
                                       << " kg of " << *commod << ".";
    }
  }
  LOG(cyc::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SinkFacility::Tock(int time) {
  LOG(cyc::LEV_INFO3, "SnkFac") << prototype() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyc::LEV_INFO4, "SnkFac") << "SinkFacility " << this->id()
                                   << " is holding " << inventory_.quantity()
                                   << " units of material at the close of month "
                                   << time << ".";
  LOG(cyc::LEV_INFO3, "SnkFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyc::Agent* ConstructSinkFacility(cyc::Context* ctx) {
  return new SinkFacility(ctx);
}
} // namespace cycamore
