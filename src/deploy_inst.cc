// Implements the DeployInst class
#include "deploy_inst.h"

namespace cycamore {

DeployInst::DeployInst(cyclus::Context* ctx) : cyclus::Institution(ctx) {}

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
  qe = qe->SubTree("config/*");

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
  for (int i = 0; i < prototypes.size(); i++) {
    std::string proto = prototypes[i];

    std::stringstream ss;
    ss << proto;

    if (lifetimes.size() == prototypes.size()) {
      cyclus::Agent* a = context()->CreateAgent<Agent>(proto);
      a->lifetime_ = lifetimes[i];

      ss << "_life" << lifetimes[i];
      proto = ss.str();
      context()->AddPrototype(proto, a);
    }

    int t = build_times[i];
    for (int j = 0; j < n_build[i]; j++) {
      context()->SchedBuild(this, proto, t);
    }
  }
}

extern "C" cyclus::Agent* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}

}  // namespace cycamore
