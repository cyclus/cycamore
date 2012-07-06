// GrowthRegion.cpp
// Implements the GrowthRegion class

#include "GrowthRegion.h"

#include "InputXML.h"
#include "Model.h"
#include "SupplyDemand.h"
#include "BuildingManager.h"
#include "SymbolicFunctions.h"

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

  xmlNodeSetPtr commodity_nodes = 
    XMLinput->get_xpath_elements(model_cur,"gcommodity");\

  for (int i=0;i<commodity_nodes->nodeNr;i++) {
    // instantiate product
    string name = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "name");
    commodities_.push_back(name);
    Commodity commodity(name);

    // instantiate demand
    string type = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "demand/type");
    string params = 
      (const char*)XMLinput->get_xpath_content(commodity_nodes->nodeTab[i], "demand/parameters");
    // FunctionPtr demand = SymbolicFunctions::getFunctionPtr(type,params); // need to add this capability to SymbolicFunctions
    FunctionPtr demand;

    // set up producers vector  
    vector<Producer> producers;
    xmlNodeSetPtr producer_nodes = 
      XMLinput->get_xpath_elements(commodity_nodes->nodeTab[i],"metby");

    for (int j=0;i<producer_nodes->nodeNr;i++){
      string fac_name = 
        (const char*)XMLinput->get_xpath_content(producer_nodes->nodeTab[j], "facility");
      double capacity = 
        atof(XMLinput->get_xpath_content(producer_nodes->nodeTab[j], "capacity"));
      Producer p(fac_name,commodity,capacity,1); // cost = 1
      producers.push_back(p);
    } // end producer nodes

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
  map<string, pair<Model*,Model*> > producers_by_name;
  
  // if there are no children, yell
  if ( children_.empty() ) {
    stringstream err("");
    err << "GrowthRegion " << this->name() << " cannot populate its list"
        << " of builders because it has no children.";
    throw CycOverrideException(err.str());
  }

  // // for each child
  // for(vector<Model*>::iterator inst = children_.begin();
  //     inst != children_.end(); inst++) {
  //   // for each prototype of that child
  //   for(PrototypeIterator 
  //         fac = (dynamic_cast<InstModel*>(*inst))->beginPrototype();
  //       fac != (dynamic_cast<InstModel*>(*inst))->endPrototype(); 
  //       fac++) {
  //     producers_by_name.insert( (*fac)->name(), pair<Model*,Model*>((*inst),(*fac)) );
  //   }  // end prototypes
  // } // end children

  // // populate the maps
  // for (CommodityIterator ci = sdmanager_.begin(); ci != sdmanager_.end(); ci++) {
    
    
  // }
  
  
  // if there are no builders, yell
  if ( builders_.empty() ) {
    stringstream err("");
    err << "BuildRegion " << this->name() << " has finished populating"
        << " its list of builders, but that list is empty.";
    throw CycOverrideException(err.str());
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
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

