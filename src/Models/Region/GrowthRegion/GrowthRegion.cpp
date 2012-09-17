// GrowthRegion.cpp
// Implements the GrowthRegion class

#include "GrowthRegion.h"

#include "Prototype.h"
#include "QueryEngine.h"
#include "SupplyDemand.h"
#include "BuildingManager.h"
#include "InstModel.h"
#include "SymbolicFunctionFactories.h"
#include "CycException.h"

#include <stdlib.h>
#include <vector>

using namespace std;
using namespace boost;

/* --------------------
 * GrowthRegion Class Methods
 * --------------------
 */
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
GrowthRegion::GrowthRegion() {
  builders_ = map<Producer*,Model*>();
  producers_ = map<Producer*,Model*>();
  commodities_ = vector<Commodity>();
  sdmanager_ = SupplyDemandManager();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::initModuleMembers(QueryEngine* qe) {
  LOG(LEV_DEBUG2, "greg") << "A Growth Region is being initialized";
  
  string query = "gcommodity";

  int nCommodities = qe->nElementsMatchingQuery(query);

  // for now we can only handle one commodity
  if (nCommodities > 1) {
    stringstream err("");
    err << "GrowthRegion can currently only handle demand for "
        << "one commodity type.";
    throw CycException(err.str());
  }

  // populate supply demand manager info for each commodity
  for (int i=0; i<nCommodities; i++) {
    initCommodity(qe->queryElement(query,i));
  }

  // instantiate building manager
  initBuildManager();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::enterSimulation(Model* parent) {
  RegionModel::enterSimulation(parent);
  // populate producers_, builders_
  populateProducerMaps();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::initBuildManager() {
  buildmanager_ = 
    shared_ptr<BuildingManager>(new BuildingManager(&sdmanager_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::initCommodity(QueryEngine* qe) {
  // instantiate product
  string name = qe->getElementContent("name");
  Commodity commodity(name);
  int position = commodities_.size();
  commodities_.push_back(commodity);
  
  // instantiate demand
  QueryEngine* demand = qe->queryElement("demand");
  string type = demand->getElementContent("type");
  string params = demand->getElementContent("parameters");
  BasicFunctionFactory bff;
  FunctionPtr demand_function = bff.getFunctionPtr(type,params);

  // set up producers vector  
  vector<Producer> producers;
  string query = "metby";
  int nProducers = demand->nElementsMatchingQuery(query);

  for (int i=0; i<nProducers; i++) {
    Producer p = 
      getProducer(demand->
                  queryElement(query,i),commodities_.at(position));
    producers.push_back(p);
  } // end producer nodes
    
    // populate info
  sdmanager_.registerCommodity(commodities_.at(position),demand_function,producers);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Producer GrowthRegion::getProducer(QueryEngine* qe,
                                   Commodity& commodity) {
  string fac_name = qe->getElementContent("facility");
  double capacity = strtol(qe->getElementContent("capacity").c_str(),NULL,10);
  double cost = capacity; // cost = capacity
  return Producer(fac_name,commodity,capacity,cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string GrowthRegion::str() {
  std::string s = RegionModel::str();
  return s;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::handleTick(int time) {

  // for each commodity
  for (int i = 0; i < commodities_.size(); i++) {
    Commodity c = commodities_.at(i);
    
    // does demand exist?
    double unmet_demand = 
      sdmanager_.supply(c) - sdmanager_.demand(c,time);
    
    // if so, determine which prototypes to build and build them
    if (unmet_demand > 0) {
      vector<BuildOrder> orders = 
        buildmanager_->makeBuildDecision(c,unmet_demand);
      // build the prototypes
      orderBuilds(orders);
    }
  }
  
  // After we finish building, call the normal handleTick for a region
  RegionModel::handleTick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::populateProducerMaps() {

  // if there are no children, yell
  if ( children_.empty() ) {
    stringstream err("");
    err << "GrowthRegion " << this->name() 
        << " cannot populate its list"
        << " of builders because it has no children.";
    throw CycOverrideException(err.str());
  }

  // for each commodity
  for (int i = 0; i < commodities_.size(); i++) {
    Commodity c = commodities_.at(i);
    
    // map each producer's name to a pointer to it
    map<string,Producer*> producer_names;
    populateProducerNames(c,producer_names);
    
    // populate the maps with those producer names
    populateMaps(this,producer_names);
  }
  
  // if there are no builders, yell
  if ( builders_.empty() ) {
    stringstream err("");
    err << "GrowthRegion " << this->name() 
        << " has finished populating"
        << " its list of builders, but that list is empty.";
    throw CycOverrideException(err.str());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::populateProducerNames(Commodity& c, 
                                         std::map<std::string,Producer*>& 
                                         producer_names) {
  for (int j = 0; j < sdmanager_.nProducers(c); j++) {
    Producer* p = sdmanager_.producer(c,j);
    producer_names[p->name()] = p;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::populateMaps(Model* node, 
                                std::map<std::string,Producer*>& 
                                producer_names) {
  // the name to search for
  string model_name = node->name();

  // if the model is in producers, log it as a producer
  // and its parent as a builder
  map<string,Producer*>::iterator it = producer_names.find(model_name);
  if (it != producer_names.end()) {
    producers_[it->second] = node;
    builders_[it->second] = node->parent();
  }
  
  // perform the same operation for each of this node's children
  for (int i = 0; i < node->nChildren(); i++) {
    populateMaps(node->children(i),producer_names);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string GrowthRegion::printMaps() {
  stringstream ss("");
  map<Producer*,Model*>::iterator it;
  ss << "Producer map:" << endl;
  for (it = producers_.begin(); it != producers_.end(); it++) {
    ss << "\t" << it->first->name() << " producer produces model " 
       << it->second->name() << endl;
  }
  ss << "Builder map:" << endl;
  for (it = builders_.begin(); it != builders_.end(); it++) {
    ss << "\t" << it->first->name() << " producer is built by model " 
       << it->second->name() << endl;
  }
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::orderBuilds(std::vector<BuildOrder>& orders) {
  // for each order
  for (int i = 0; i < orders.size(); i++) {
    BuildOrder bo = orders.at(i);
    // for each instance of a prototype order
    for (int j = 0; j < bo.number; j++) {
      Model* builder = builders_[bo.producer];
      Model* prototype = producers_[bo.producer];
      orderBuild(builder,prototype);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::orderBuild(Model* builder, Model* prototype) {
  // build functions must know who is placing the build order
  dynamic_cast<InstModel*>(builder)->build(dynamic_cast<Prototype*>(prototype));
}
/* -------------------- */


/* --------------------
 * Model Class Methods
 * --------------------
 */
extern "C" Model* constructGrowthRegionGrowthRegion() {
    return new GrowthRegion();
}

/* -------------------- */

