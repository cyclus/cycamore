// Implements the Sink class
#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "sink.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sink::Sink(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      capacity(std::numeric_limits<double>::max()),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longitude) {
  SetMaxInventorySize(std::numeric_limits<double>::max());}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sink::~Sink() {}

#pragma cyclus def schema cycamore::Sink

#pragma cyclus def annotations cycamore::Sink

#pragma cyclus def infiletodb cycamore::Sink

#pragma cyclus def snapshot cycamore::Sink

#pragma cyclus def snapshotinv cycamore::Sink

#pragma cyclus def initinv cycamore::Sink

#pragma cyclus def clone cycamore::Sink

#pragma cyclus def initfromdb cycamore::Sink

#pragma cyclus def initfromcopy cycamore::Sink

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::EnterNotify() {
  cyclus::Facility::EnterNotify();
  LOG(cyclus::LEV_INFO4, "SnkFac") << " using random behavior " << random_size;

  if (in_commod_prefs.size() == 0) {
    for (int i = 0; i < in_commods.size(); ++i) {
      in_commod_prefs.push_back(cyclus::kDefaultPref);
    }
  } else if (in_commod_prefs.size() != in_commods.size()) {
    std::stringstream ss;
    ss << "in_commod_prefs has " << in_commod_prefs.size()
       << " values, expected " << in_commods.size();
    throw cyclus::ValueError(ss.str());
  }
  /// Create first requestAmt. Only used in testing, as a simulation will
  /// overwrite this on Tick()
  SetRequestAmt();

  RecordPosition();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Sink::str() {
  using std::string;
  using std::vector;
  std::stringstream ss;
  ss << cyclus::Facility::str();

  string msg = "";
  msg += "accepts commodities ";
  for (vector<string>::iterator commod = in_commods.begin();
       commod != in_commods.end();
       commod++) {
    msg += (commod == in_commods.begin() ? "{" : ", ");
    msg += (*commod);
  }
  msg += "} until its inventory is full at ";
  ss << msg << inventory.capacity() << " kg.";
  return "" + ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
Sink::GetMatlRequests() {
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  using cyclus::Composition;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat;

  /// for testing
  if (requestAmt > SpaceAvailable()) {
    SetRequestAmt();
  }

  if (recipe_name.empty()) {
    mat = cyclus::NewBlankMaterial(requestAmt);
  } else {
    Composition::Ptr rec = this->context()->GetRecipe(recipe_name);
    mat = cyclus::Material::CreateUntracked(requestAmt, rec);
  }

  if (requestAmt > cyclus::eps()) {  
    std::vector<Request<Material>*> mutuals;
    for (int i = 0; i < in_commods.size(); i++) {
      mutuals.push_back(port->AddRequest(mat, this, in_commods[i], in_commod_prefs[i]));

    }
    port->AddMutualReqs(mutuals);
    ports.insert(port);
  }  // if amt > eps
  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
Sink::GetGenRsrcRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Product;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Product>::Ptr> ports;
  RequestPortfolio<Product>::Ptr
      port(new RequestPortfolio<Product>());

  if (requestAmt > cyclus::eps()) {
    CapacityConstraint<Product> cc(requestAmt);
    port->AddConstraint(cc);

    std::vector<std::string>::const_iterator it;
    for (it = in_commods.begin(); it != in_commods.end(); ++it) {
      std::string quality = "";  // not clear what this should be..
      Product::Ptr rsrc = Product::CreateUntracked(requestAmt, quality);
      port->AddRequest(rsrc, this, *it);
    }

    ports.insert(port);
  }  // if amt > eps
  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory.Push(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::AcceptGenRsrcTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Product>,
                                 cyclus::Product::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Product>,
                         cyclus::Product::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory.Push(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::Tick() {
  using std::string;
  using std::vector;
  LOG(cyclus::LEV_INFO3, "SnkFac") << prototype() << " is ticking {";

  SetRequestAmt();

  LOG(cyclus::LEV_INFO3, "SnkFac") << prototype() << " has default request amount " << requestAmt;

  // inform the simulation about what the sink facility will be requesting
  if (requestAmt > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "SnkFac") << prototype()
                                       << " has request amount " << requestAmt
                                       << " kg of " << in_commods[0] << ".";
    for (vector<string>::iterator commod = in_commods.begin();
         commod != in_commods.end();
         commod++) {
      LOG(cyclus::LEV_INFO4, "SnkFac") << " will request " << requestAmt
                                       << " kg of " << *commod << ".";
      cyclus::toolkit::RecordTimeSeries<double>("demand"+*commod, this,
                                            requestAmt);
    }
  }
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::Tock() {
  LOG(cyclus::LEV_INFO3, "SnkFac") << prototype() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SnkFac") << "Sink " << this->id()
                                   << " is holding " << inventory.quantity()
                                   << " units of material at the close of timestep "
                                   << context()->time() << ".";
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

void Sink::RecordPosition() {
  std::string specification = this->spec();
  context()
      ->NewDatum("AgentPosition")
      ->AddVal("Spec", specification)
      ->AddVal("Prototype", this->prototype())
      ->AddVal("AgentId", id())
      ->AddVal("Latitude", latitude)
      ->AddVal("Longitude", longitude)
      ->Record();
}

void Sink::SetRequestAmt() {
  double amt = SpaceAvailable();
  if (amt < cyclus::eps()) {
    requestAmt = 0;
  }

  if (random_size == "None") {
    requestAmt =  amt;
  }
  else if (random_size == "UniformReal") {
    requestAmt =  context()->random_uniform_real(0, amt);
  }
  else if (random_size == "NormalReal") {
    requestAmt =  context()->random_normal_real(amt * random_size_mean,
                                         amt * random_size_stddev, 0, 
                                         amt);
  }
  else {
    requestAmt =  amt;
  }
  return;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSink(cyclus::Context* ctx) {
  return new Sink(ctx);
}

}  // namespace cycamore
