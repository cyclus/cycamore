// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

#include "error.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DeployInst::DeployInst(cyclus::Context* ctx)
    : cyclus::InstModel(ctx) {}

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
  qe = qe->QueryElement("model/" + model_impl());

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
    for (int j = 0; j < number; ++j) {
      build_sched_[time].push_back(name);
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DeployInst::Build(cyclus::Model* parent) {
  cyclus::InstModel::Build(parent);
  BuildSched::iterator it;
  for (it = build_sched_.begin(); it != build_sched_.end(); ++it) {
    int t = it->first;
    std::vector<std::string> protos = it->second;
    for (int i = 0; i < protos.size(); ++i) {
      context()->SchedBuild(this, protos[i], t);
    }
  }
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
