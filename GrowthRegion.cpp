// GrowthRegion.cpp
// Implements the GrowthRegion class

#include "GrowthRegion.h"

#include "InputXML.h"
#include "Model.h"
#include "SupplyDemand.h"
#include "BuildingManager.h"
#include "SymbolicFunctionFactories.h"
#include "CycException.h"

#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

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
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::init(xmlNodePtr cur) {
  LOG(LEV_DEBUG2, "greg") << "A Growth Region is being initialized";
  // xml inits
  Model::init(cur); // name_ and model_impl_
  RegionModel::initAllowedFacilities(cur); // allowedFacilities_

  // get path to this model
  xmlNodePtr model_cur = 
    XMLinput->get_xpath_element(cur,"model/GrowthRegion");

  // initialize supply demand manager
  sdmanager_ = SupplyDemandManager();

  // get all commodities
  xmlNodeSetPtr commodity_nodes = 
    XMLinput->get_xpath_elements(model_cur,"gcommodity");\

  // for now we can only handle one commodity
  if (commodity_nodes->nodeNr > 1) {
    stringstream err("");
    err << "GrowthRegion can currently only handle demand for "
        << "one commodity type.";
    throw CycException(err.str());
  }

  // populate supply demand manager info for each commodity
  for (int i=0;i<commodity_nodes->nodeNr;i++) {
    // instantiate product
    string name = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "name");
    Commodity commodity(name);
    commodities_.push_back(commodity);

    // instantiate demand
    string type = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "demand/type");
    string params = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "demand/parameters");
    BasicFunctionFactory bff;
    FunctionPtr demand = bff.getFunctionPtr(type,params);

    // set up producers vector  
    vector<Producer> producers;
    xmlNodeSetPtr producer_nodes = 
      XMLinput->get_xpath_elements(commodity_nodes->nodeTab[i],"demand/metby");

    for (int j=0;i<producer_nodes->nodeNr;i++){
      string fac_name = 
        (const char*)XMLinput->get_xpath_content(producer_nodes->nodeTab[j], "facility");
      double capacity = 
        atof(XMLinput->get_xpath_content(producer_nodes->nodeTab[j], "capacity"));
      Producer p(fac_name,commodity,capacity,1); // cost = 1
      producers.push_back(p);
    } // end producer nodes

    // populate info
    sdmanager_.registerCommodity(commodity,demand,producers);
  } // end commodity nodes

  // instantiate building manager
  buildmanager_ = shared_ptr<BuildingManager>(new BuildingManager(sdmanager_));
  
  // parent_ and tick listener, model 'born'
  RegionModel::initSimInteraction(this); 
  // children->setParent, requires init()
  RegionModel::initChildren(cur); 

  // populate producers_, builders_
  populateProducerMaps();
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::copy(GrowthRegion* src) {
  RegionModel::copy(src);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string GrowthRegion::str() {
  std::string s = RegionModel::str();

  // if ( builders_ == NULL || builders_->empty() ){
  //   s += name() + " has no builders (currently)."; 
  // } else {
  //   s += name() + " has the following builders: " ; 
  //   for(map<Model*, list<Model*>*>::iterator mit=builders_->begin();
  //       mit != builders_->end(); mit++) {
  //     s += " prototype=" + mit->first->name() + "("; 
  //     for(list<Model*>::iterator inst = mit->second->begin();
  //         inst != mit->second->end(); inst++) {
  //       s += (*inst)->name() + ", "; 
  //     }
  //     s += "), ";
  //   }
  // }
  return s;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::handleTick(int time) {
  // After we finish building, call the normal handleTick for a region
  RegionModel::handleTick(time);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::populateProducerMaps() {

  // if there are no children, yell
  if ( children_.empty() ) {
    stringstream err("");
    err << "GrowthRegion " << this->name() << " cannot populate its list"
        << " of builders because it has no children.";
    throw CycOverrideException(err.str());
  }

  // for each commodity
  for (int i = 0; i < commodities_.size(); i++) {
    Commodity c = commodities_.at(i);
    
    // map each producer's name to a pointer to it
    map<string,Producer*> producer_names;
    for (int j = 0; 
         j < sdmanager_.nProducers(c); 
         j++) {
      Producer* p = sdmanager_.producer(c,j);
      producer_names[p->name()]=p;
    }
    
    // populate the maps with those producer names
    populateMaps(this,producer_names);
  }
  
  // if there are no builders, yell
  if ( builders_.empty() ) {
    stringstream err("");
    err << "BuildRegion " << this->name() << " has finished populating"
        << " its list of builders, but that list is empty.";
    throw CycOverrideException(err.str());
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
  if (producer_names.find(model_name) != producer_names.end()) {
    producers_[producer_names[model_name]] = node;
    builders_[producer_names[model_name]] = node->parent();
  }
  
  // perform the same operation for this node's children
  for (int i = 0; i < node->nChildren(); i++) {
    populateMaps(node->children(i),producer_names);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void GrowthRegion::orderBuild(Model* builder, Model* buildee) {
}
/* -------------------- */


/* --------------------
 * Model Class Methods
 * --------------------
 */

extern "C" Model* constructGrowthRegion() {
    return new GrowthRegion();
}

/* -------------------- */

