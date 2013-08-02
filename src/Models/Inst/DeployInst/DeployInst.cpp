// DeployInst.cpp
// Implements the DeployInst class

#include "DeployInst.h"

#include "CycException.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void BuildOrderList::addBuildOrder(cyclus::Prototype* p, int number, 
                                   int time) {
  map<int, set<BuildOrder> >::iterator it;
  it = all_orders_.find(time);

  if (it == all_orders_.end()) {
    set<BuildOrder> orders;
    orders.insert(make_pair(p,number));
    all_orders_.insert(make_pair(time,orders));
  } else {
    it->second.insert(make_pair(p,number));
  }  

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::set<BuildOrder> BuildOrderList::extractOrders(int time) {
  map<int, set<BuildOrder> >::iterator it;
  set<BuildOrder> orders;
  it = all_orders_.find(time);
  if (it != all_orders_.end()) {
    orders = it->second;
    all_orders_.erase(it);
  }
  return orders;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
DeployInst::DeployInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
DeployInst::~DeployInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void DeployInst::initModuleMembers(cyclus::QueryEngine* qe) {
  string query = "buildorder";
  int nOrders = qe->nElementsMatchingQuery(query);
  
  for (int i = 0; i < nOrders; i++) {
    cyclus::QueryEngine* order = qe->queryElement(query,i);
    string name = order->getElementContent("prototype");
    int number = atoi(order->getElementContent("number").c_str());
    int time = atoi(order->getElementContent("date").c_str());
    build_orders_.addBuildOrder(cyclus::Prototype::getRegisteredPrototype(name),
                                number,time);
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
void DeployInst::handleTick(int time) {
  set<BuildOrder> orders = build_orders_.extractOrders(time);
  for (set<BuildOrder>::iterator it = orders.begin(); 
       it != orders.end(); it++) {
    
    cyclus::Prototype* p = it->first;
    int number = it->second;
    
    for (int i = 0; i < number; i++) {
      // build as many as required
      build(p);
    }

  }
  InstModel::handleTick(time);
}

/* ------------------- */ 


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructDeployInst() {
  return new DeployInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructDeployInst(cyclus::Model* model) {
  delete model;
}

/* ------------------- */ 

