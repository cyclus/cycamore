// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

#include "error.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BuildOrderList::AddBuildOrder(cyclus::Prototype* p, int number,
                                   int time) {
  using std::map;
  using std::set;
  using std::make_pair;
  map<int, set<BuildOrder> >::iterator it;
  it = all_orders_.find(time);

  if (it == all_orders_.end()) {
    set<BuildOrder> orders;
    orders.insert(make_pair(p, number));
    all_orders_.insert(make_pair(time, orders));
  } else {
    it->second.insert(make_pair(p, number));
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<BuildOrder> BuildOrderList::ExtractOrders(int time) {
  using std::map;
  using std::set;
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
void DeployInst::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  string query = "buildorder";
  int nOrders = qe->NElementsMatchingQuery(query);

  for (int i = 0; i < nOrders; i++) {
    cyclus::QueryEngine* order = qe->QueryElement(query, i);
    string name = order->GetElementContent("prototype");
    int number = atoi(order->GetElementContent("number").c_str());
    int time = atoi(order->GetElementContent("date").c_str());
    build_orders_.AddBuildOrder(cyclus::Prototype::GetRegisteredPrototype(name),
                                number, time);
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DeployInst::HandleTick(int time) {
  using std::set;
  set<BuildOrder> orders = build_orders_.ExtractOrders(time);
  for (set<BuildOrder>::iterator it = orders.begin();
       it != orders.end(); it++) {

    cyclus::Prototype* p = it->first;
    int number = it->second;

    for (int i = 0; i < number; i++) {
      // build as many as required
      Build(p);
    }

  }
  InstModel::HandleTick(time);
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

