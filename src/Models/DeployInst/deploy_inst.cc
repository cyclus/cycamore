// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

#include "error.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BuildOrderList::AddBuildOrder(std::string prototype, int number,
                                   int time) {
  using std::map;
  using std::set;
  using std::make_pair;
  map<int, set<BuildOrder> >::iterator it;
  it = all_orders_.find(time);

  if (it == all_orders_.end()) {
    set<BuildOrder> orders;
    orders.insert(make_pair(prototype, number));
    all_orders_.insert(make_pair(time, orders));
  } else {
    it->second.insert(make_pair(prototype, number));
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<BuildOrder> BuildOrderList::ExtractOrders(int time) {
  using std::map;
  using std::set;
  using std::make_pair;
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
DeployInst::DeployInst(cyclus::Context* ctx)
    : cyclus::InstModel(ctx),
      cyclus::Model(ctx) {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DeployInst::~DeployInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string DeployInst::schema() {
  return
    "<oneOrMore>                               \n"
    "	<element name=\"buildorder\">            \n"
    "	  <element name=\"prototype\">           \n"
    "	    <data type=\"string\"/>              \n"
    "	  </element>                             \n"
    "	  <element name=\"number\">              \n"
    "	    <data type=\"nonNegativeInteger\"/>  \n"
    "	  </element>                             \n"
    "	  <element name=\"date\">                \n"
    "	    <data type=\"nonNegativeInteger\"/>  \n"
    "	  </element>                             \n"
    "	</element>                               \n"
    "</oneOrMore>                              \n";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DeployInst::InitFrom(cyclus::QueryEngine* qe) {
  cyclus::InstModel::InitFrom(qe);
  qe = qe->QueryElement("model/" + ModelImpl());

  using std::map;
  using std::string;
  using std::make_pair;
  string query = "buildorder";

  int nOrders = qe->NElementsMatchingQuery(query);

  for (int i = 0; i < nOrders; i++) {
    cyclus::QueryEngine* order = qe->QueryElement(query, i);
    string name = order->GetElementContent("prototype");
    int number = atoi(order->GetElementContent("number").c_str());
    int time = atoi(order->GetElementContent("date").c_str());
    build_orders_.AddBuildOrder(name, number, time);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DeployInst::Tick(int time) {
  using std::map;
  using std::set;
  using std::make_pair;
  set<BuildOrder> orders = build_orders_.ExtractOrders(time);
  for (set<BuildOrder>::iterator it = orders.begin();
       it != orders.end(); it++) {

    std::string prototype = it->first;
    int number = it->second;
    for (int i = 0; i < number; i++) {
      // build as many as required
      Build(prototype);
    }
  }
  InstModel::Tick(time);
}

/* ------------------- */


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}
/* ------------------- */

} // namespace cycamore
