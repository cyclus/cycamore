// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

namespace cycamore {

DeployInst::DeployInst(cyc::Context* ctx)
    : cyc::InstModel(ctx) {}

DeployInst::~DeployInst() {}

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

void DeployInst::InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di) {
  cyc::InstModel::InfileToDb(qe, di);
  qe = qe->QueryElement("model/" + model_impl());

  int nOrders = qe->NElementsMatchingQuery("buildorder");
  for (int i = 0; i < nOrders; i++) {
    cyc::QueryEngine* order = qe->QueryElement("buildorder", i);
    int n = order->GetInt("number");
    for (int j = 0; j < n; ++j) {
      di.NewDatum("BuildOrder")
        ->AddVal("prototype", order->GetString("prototype"))
        ->AddVal("date", order->GetInt("date"))
        ->Record();
    }
  }
}

void DeployInst::InitFrom(cyc::QueryBackend* b) {
  cyc::InstModel::InitFrom(b);
  cyc::QueryResult qr = b->Query("BuildOrder", NULL);
  for (int i = 0; i < qr.rows.size(); i++) {
    std::string proto = qr.GetVal<std::string>("prototype", i);
    int t = qr.GetVal<int>("date", i);
    build_sched_[t].push_back(proto);
  }
}

void DeployInst::Snapshot(cyc::DbInit di) {
  cyc::InstModel::Snapshot(di);

  BuildSched::iterator it;
  for (it = build_sched_.begin(); it != build_sched_.end(); ++it) {
    int t = it->first;
    std::vector<std::string> protos = it->second;
    for (int i = 0; i < protos.size(); ++i) {
      di.NewDatum("BuildOrder")
        ->AddVal("prototype", protos[i])
        ->AddVal("date", t)
        ->Record();
    }
  }
}

void DeployInst::Build(cyc::Model* parent) {
  cyc::InstModel::Build(parent);
  BuildSched::iterator it;
  for (it = build_sched_.begin(); it != build_sched_.end(); ++it) {
    int t = it->first;
    std::vector<std::string> protos = it->second;
    for (int i = 0; i < protos.size(); ++i) {
      context()->SchedBuild(this, protos[i], t);
    }
  }
}

extern "C" cyc::Model* ConstructDeployInst(cyc::Context* ctx) {
  return new DeployInst(ctx);
}

} // namespace cycamore
