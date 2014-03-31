// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

namespace cycamore {

DeployInst::DeployInst(cyclus::Context* ctx)
    : cyclus::Institution(ctx) {}

DeployInst::~DeployInst() {}

#pragma cyclus def clone cycamore::DeployInst

std::string DeployInst::schema() {
    return
        "<oneOrMore>                               \n"
        "<element name=\"buildorder\">            \n"
        "  <element name=\"prototype\">           \n"
        "    <data type=\"string\"/>              \n"
        "  </element>                             \n"
        "  <element name=\"number\">              \n"
        "    <data type=\"nonNegativeInteger\"/>  \n"
        "  </element>                             \n"
        "  <element name=\"date\">                \n"
        "    <data type=\"nonNegativeInteger\"/>  \n"
        "  </element>                             \n"
        "</element>                               \n"
        "</oneOrMore>                              \n";
}

void DeployInst::InfileToDb(cyclus::InfileTree* qe, cyclus::DbInit di) {
  cyclus::Institution::InfileToDb(qe, di);
  qe = qe->SubTree("agent/" + agent_impl());

  int nOrders = qe->NMatches("buildorder");
  for (int i = 0; i < nOrders; i++) {
    cyclus::InfileTree* order = qe->SubTree("buildorder", i);
    int n = cyclus::Query<int>(order, "number");
    for (int j = 0; j < n; ++j) {
      di.NewDatum("BuildOrder")
          ->AddVal("prototype", order->GetString("prototype"))
          ->AddVal("date", cyclus::Query<int>(order, "date"))
          ->Record();
    }
  }
}

void DeployInst::InitFrom(cyclus::QueryableBackend* b) {
  cyclus::Institution::InitFrom(b);
  cyclus::QueryResult qr = b->Query("BuildOrder", NULL);
  for (int i = 0; i < qr.rows.size(); i++) {
    std::string proto = qr.GetVal<std::string>("prototype", i);
    int t = qr.GetVal<int>("date", i);
    build_sched_[t].push_back(proto);
  }
}

void DeployInst::InitFrom(DeployInst* m) {
  cyclus::Institution::InitFrom(m);
  build_sched_ = m->build_sched_;
}

void DeployInst::Snapshot(cyclus::DbInit di) {
  cyclus::Institution::Snapshot(di);

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

void DeployInst::Build(cyclus::Agent* parent) {
  cyclus::Institution::Build(parent);
  BuildSched::iterator it;
  for (it = build_sched_.begin(); it != build_sched_.end(); ++it) {
    int t = it->first;
    std::vector<std::string> protos = it->second;
    for (int i = 0; i < protos.size(); ++i) {
      context()->SchedBuild(this, protos[i], t);
    }
  }
}

extern "C" cyclus::Agent* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}

} // namespace cycamore
