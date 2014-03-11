// deploy_inst.cc
// Implements the DeployInst class

#include "deploy_inst.h"

#include "error.h"

namespace cycamore {

DeployInst::DeployInst(cyclus::Context* ctx)
    : cyclus::InstModel(ctx) {}

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
    string name = order->GetString("prototype");
    int number = atoi(order->GetString("number").c_str());
    int time = atoi(order->GetString("date").c_str());
    for (int j = 0; j < number; ++j) {
      build_sched_[time].push_back(name);
    }
  }
}

void DeployInsg::InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di) {
  cyc::InstModel::InfileToDb(qe, di);
  qe = qe->QueryElement("model/" + model_impl());

  int nOrders = qe->NElementsMatchingQuery("buildorder");
  for (int i = 0; i < nOrders; i++) {
    cyclus::QueryEngine* order = qe->QueryElement("buildorder", i);
    string name = order->GetString("prototype");
    int number = atoi(order->GetString("number").c_str());
    int time = atoi(order->GetString("date").c_str());
    for (int j = 0; j < number; ++j) {
      build_sched_[time].push_back(name);
    }
  }

  
  using std::numeric_limits;
  using boost::lexical_cast;
  cyc::QueryEngine* output = qe->QueryElement("output");

  std::string recipe = output->GetString("recipe");
  std::string out_commod = output->GetString("outcommodity");
  double cap = cyc::GetOptionalQuery<double>(output,
                                                "output_capacity",
                                                numeric_limits<double>::max());
  di.NewDatum("Info")
    ->AddVal("recipe", recipe)
    ->AddVal("out_commod", out_commod)
    ->AddVal("capacity", cap)
    ->AddVal("curr_capacity", cap)
    ->Record();
}

void DeployInsg::InitFrom(cyc::QueryBackend* b) {
  cyc::FacilityModel::InitFrom(b);
  cyc::QueryResult qr = b->Query("Info", NULL);
  recipe_name_ = qr.GetVal<std::string>("recipe");
  out_commod_ = qr.GetVal<std::string>("out_commod");
  capacity_ = qr.GetVal<double>("capacity");
  current_capacity_ = qr.GetVal<double>("curr_capacity");

  cyc::Commodity commod(out_commod_);
  cyc::CommodityProducer::AddCommodity(commod);
  cyc::CommodityProducer::SetCapacity(commod, capacity_);
}

void DeployInsg::Snapshot(cyc::DbInit di) {
  cyc::FacilityModel::Snapshot(di);
  di.NewDatum("Info")
    ->AddVal("recipe", recipe_name_)
    ->AddVal("out_commod", out_commod_)
    ->AddVal("capacity", capacity_)
    ->AddVal("curr_capacity", current_capacity_)
    ->Record();
}

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

extern "C" cyclus::Model* ConstructDeployInst(cyclus::Context* ctx) {
  return new DeployInst(ctx);
}

} // namespace cycamore
