// batch_reactor.cc
// Implements the BatchReactor class
#include <sstream>
#include <cmath>

#include <boost/lexical_cast.hpp>

#include "cyc_limits.h"
#include "context.h"
#include "error.h"
#include "logger.h"

#include "batch_reactor.h"

namespace cycamore {

// static members
std::map<BatchReactor::Phase, std::string> BatchReactor::phase_names_ =
    std::map<Phase, std::string>();

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BatchReactor::BatchReactor(cyclus::Context* ctx)
    : cyclus::FacilityModel(ctx),
      cyclus::Model(ctx),
      process_time_(1),
      preorder_time_(0),
      refuel_time_(0),
      start_time_(-1),
      n_batches_(1),
      n_load_(1),
      n_reserves_(1),
      batch_size_(1),
      in_commodity_(""),
      in_recipe_(""),
      out_commodity_(""),
      out_recipe_(""),
      phase_(INITIAL) {
  reserves_.set_capacity(cyclus::kBuffInfinity);
  core_.set_capacity(cyclus::kBuffInfinity);
  storage_.set_capacity(cyclus::kBuffInfinity);
  if (phase_names_.empty()) {
    SetUpPhaseNames_();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BatchReactor::~BatchReactor() {}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string BatchReactor::schema() {
  return
    "  <!-- cyclus::Material In/Out  -->         \n"
    "  <element name=\"fuel_input\">             \n"
    "   <ref name=\"incommodity\"/>              \n"
    "   <ref name=\"inrecipe\"/>                 \n"
    "  </element>                                \n"
    "  <element name=\"fuel_output\">            \n"
    "   <ref name=\"outcommodity\"/>             \n"
    "   <ref name=\"outrecipe\"/>                \n"
    "  </element>                                \n"
    "                                            \n"
    "  <!-- Facility Parameters -->              \n"
    "  <element name=\"processtime\">            \n"
    "    <data type=\"nonNegativeInteger\"/>     \n"
    "  </element>                                \n"
    "  <element name=\"nbatches\">               \n"
    "    <data type=\"nonNegativeInteger\"/>     \n"
    "  </element>                                \n"
    "  <element name =\"batchsize\">             \n"
    "    <data type=\"double\"/>                 \n"
    "  </element>                                \n"
    "  <optional>                                \n"
    "    <element name =\"refueltime\">          \n"
    "      <data type=\"nonNegativeInteger\"/>   \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "  <optional>                                \n"
    "    <element name =\"orderlookahead\">      \n"
    "      <data type=\"nonNegativeInteger\"/>   \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "  <optional>                                \n"
    "    <element name =\"norder\">              \n"
    "      <data type=\"nonNegativeInteger\"/>   \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "  <optional>                                \n"
    "    <element name =\"nreload\">             \n"
    "      <data type=\"nonNegativeInteger\"/>   \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "                                            \n"
    "  <!-- Power Production  -->                \n"
    "  <element name=\"commodity_production\">   \n"
    "   <element name=\"commodity\">             \n"
    "     <data type=\"string\"/>                \n"
    "   </element>                               \n"
    "   <element name=\"capacity\">              \n"
    "     <data type=\"double\"/>                \n"
    "   </element>                               \n"
    "   <element name=\"cost\">                  \n"
    "     <data type=\"double\"/>                \n"
    "   </element>                               \n"
    "  </element>                                \n";
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  using boost::lexical_cast;

  // in/out
  cyclus::QueryEngine* input = qe->QueryElement("fuel_input");
  in_commodity(input->GetElementContent("incommodity"));
  in_recipe(input->GetElementContent("inrecipe"));
  
  cyclus::QueryEngine* output = qe->QueryElement("fuel_output");
  out_commodity(output->GetElementContent("outcommodity"));
  out_recipe(output->GetElementContent("outrecipe"));

  // facility data required
  string data;
  data = qe->GetElementContent("processtime");
  process_time(lexical_cast<int>(data));
  data = qe->GetElementContent("nbatches");
  n_batches(lexical_cast<int>(data));
  data = qe->GetElementContent("batchsize");
  batch_size(lexical_cast<double>(data));

  // facility data optional  
  int time =
      cyclus::GetOptionalQuery<int>(qe, "refueltime", refuel_time());
  refuel_time(time);
  time =
      cyclus::GetOptionalQuery<int>(qe, "orderlookahead", preorder_time());
  preorder_time(time);

  int n = 
      cyclus::GetOptionalQuery<int>(qe, "nreload", n_load());
  n_load(n);
  n = cyclus::GetOptionalQuery<int>(qe, "norder", n_load());
  n_reserves(n);

  // commodity production
  cyclus::QueryEngine* commodity = qe->QueryElement("commodity_production");
  cyclus::Commodity commod(commodity->GetElementContent("commodity"));
  AddCommodity(commod);
  data = commodity->GetElementContent("capacity");
  cyclus::CommodityProducer::SetCapacity(commod,
                                         lexical_cast<double>(data));
  data = commodity->GetElementContent("cost");
  cyclus::CommodityProducer::SetCost(commod,
                                     lexical_cast<double>(data));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* BatchReactor::Clone() {
  BatchReactor* m = new BatchReactor(context());
  m->InitFrom(this);

  // in/out
  m->in_commodity(in_commodity());
  m->out_commodity(out_commodity());
  m->in_recipe(in_recipe());
  m->out_recipe(out_recipe());

  // facility params
  m->process_time(process_time());
  m->preorder_time(preorder_time());
  m->refuel_time(refuel_time());
  m->n_batches(n_batches());
  m->n_load(n_load());
  m->n_reserves(n_reserves());
  m->batch_size(batch_size());

  // commodity production
  m->CopyProducedCommoditiesFrom(this);

  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string BatchReactor::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str();
  ss << " has facility parameters {"
     << ", Process Time = " << process_time()
     << ", Refuel Time = " << refuel_time()
     << ", Core Loading = " << n_batches() * batch_size()
     << ", Batches Per Core = " << n_batches()
     << ", converts commodity '" << in_commodity()
     << "' into commodity '" << out_commodity()
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::Deploy(cyclus::Model* parent) {
  FacilityModel::Deploy(parent);
  phase(INITIAL);

  LOG(cyclus::LEV_DEBUG2, "BReact") << "Batch Reactor entering the simuluation";
  LOG(cyclus::LEV_DEBUG2, "BReact") << str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "BReact") << name() << " is ticking at time "
                                   << time << " {";
  LOG(cyclus::LEV_DEBUG3, "BReact") << "The current phase is: "
                                    << phase_names_[phase_];

  switch (phase()) {
    case PROCESS:
      if (time == end_time()) {
        for (int i = 0; i < n_load(); i++) {
          MoveBatchOut_();
        }
        phase(WAITING);
      }
      break;

    case WAITING:
      if (n_core() == n_batches() &&
          end_time() + refuel_time() <= context()->time()) {
        phase(PROCESS);
      } 
      break;
      
    case INITIAL:
      // special case for a core primed to go
      if (n_core() == n_batches()) phase(PROCESS);
      break;
  }
  
  LOG(cyclus::LEV_INFO3, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "BReact") << name() << " is tocking {";
  LOG(cyclus::LEV_DEBUG3, "BReact") << "The current phase is: "
                                    << phase_names_[phase_];
  switch (phase()) {
    case INITIAL: // falling through
    case WAITING:
      Refuel_();
      break;
  }
  LOG(cyclus::LEV_INFO3, "BReact") << "}";
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
BatchReactor::GetMatlRequests() {
  using cyclus::RequestPortfolio;
  using cyclus::Material;
  
  std::set<RequestPortfolio<Material>::Ptr> set;
  double order_size;

  switch (phase()) {
    // the initial phase requests as much fuel as necessary to achieve an entire
    // core
    case INITIAL:
      order_size = n_batches() * batch_size()
                   - core_.quantity() - reserves_.quantity();
      if (preorder_time() == 0) order_size += batch_size() * n_reserves();
      if (order_size > 0) {
        RequestPortfolio<Material>::Ptr p = GetOrder_(order_size);
        set.insert(p);
      }
      break;

    // the default case is to request the reserve amount if the order time has
    // been reached
    default:
      order_size = n_reserves() * batch_size() - reserves_.quantity();
      if (order_time() <= context()->time() &&
          order_size > 0) {
        RequestPortfolio<Material>::Ptr p = GetOrder_(order_size);
        set.insert(p);
      }
      break;
  }

  return set;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  cyclus::Material::Ptr mat = responses.at(0).second;
  for (int i = 1; i < responses.size(); i++) {
    mat->Absorb(responses.at(i).second);
  }
  AddBatches_(mat);
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
BatchReactor::GetMatlBids(const cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
  
  const std::vector<Request<Material>::Ptr>& requests =
      commod_requests.at(out_commodity_);

  std::set<BidPortfolio<Material>::Ptr> ports;
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  std::vector<Request<Material>::Ptr>::const_iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    const Request<Material>::Ptr req = *it;
    double qty = req->target()->quantity();
    if (qty <= storage_.quantity()) {
      Material::Ptr offer =
          Material::CreateUntracked(qty, context()->GetRecipe(out_recipe_));
      port->AddBid(req, offer, this);
    }
  }

  CapacityConstraint<Material> cc(storage_.quantity());
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;
  using cyclus::ResCast;

  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    LOG(cyclus::LEV_INFO5, "BReact") << name() << " just received an order.";

    double qty = it->amt;
    
    // pop amount from inventory and blob it into one material
    std::vector<Material::Ptr> manifest =
        ResCast<Material>(storage_.PopQty(qty));  
    Material::Ptr response = manifest[0];
    for (int i = 1; i < manifest.size(); i++) {
      response->Absorb(manifest[i]);
    }

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "BatchReactor") << name()
                                           << " just received an order"
                                           << " for " << qty
                                           << " of " << out_commodity_;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::phase(BatchReactor::Phase p) {
  LOG(cyclus::LEV_DEBUG2, "BReact") << "BatchReactor " << name()
                                    << " is changing phases -";
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * from phase: " << phase_names_[phase_];
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * to phase: " << phase_names_[p];
  
  switch (p) {
    case PROCESS:
      start_time(context()->time());
  }
  phase_ = p;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::Refuel_() {
  while(n_core() < n_batches() && BatchIn(reserves_, batch_size())) {
    MoveBatchIn_();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::MoveBatchIn_() {
  LOG(cyclus::LEV_DEBUG2, "BReact") << "BatchReactor " << name() << " added"
                                    <<  " a batch from its core.";
  core_.Push(reserves_.Pop());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::MoveBatchOut_() {
  using cyclus::Material;
  using cyclus::ResCast;
  
  LOG(cyclus::LEV_DEBUG2, "BReact") << "BatchReactor " << name() << " removed"
                                    <<  " a batch from its core.";

  Material::Ptr mat = ResCast<Material>(core_.Pop());
  mat->Transmute(context()->GetRecipe(out_recipe()));
  storage_.Push(mat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RequestPortfolio<cyclus::Material>::Ptr
BatchReactor::GetOrder_(double size) {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  LOG(cyclus::LEV_DEBUG3, "BReact") << "BatchReactor " << name()
                                    << " is making an order of size: "
                                    << size;

  Material::Ptr mat =
      Material::CreateUntracked(size, context()->GetRecipe(in_recipe_));
  
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  port->AddRequest(mat, this, in_commodity_);

  CapacityConstraint<Material> cc(size);
  port->AddConstraint(cc);

  return port;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::AddBatches_(cyclus::Material::Ptr mat) {
  using cyclus::Material;
  using cyclus::ResCast;

  LOG(cyclus::LEV_DEBUG3, "BReact") << "BatchReactor " << name()
                                    << " is adding " << mat->quantity()
                                    << " of material to its reserves.";

  if (reserves_.count() > 0) {
    Material::Ptr last = ResCast<Material>(reserves_.PopBack());
    if (last->quantity() < batch_size()) {
      if (last->quantity() + mat->quantity() <= batch_size()) {
        last->Absorb(mat);
        reserves_.Push(last);
        return; // return because mat has been absorbed
      } else {
        Material::Ptr last_bit = mat->ExtractQty(batch_size() - last->quantity());
        last->Absorb(last_bit);
      } // end if (last->quantity() + mat->quantity() <= batch_size())
    } // end if (last->quantity() < batch_size())
    reserves_.Push(last);
  } // end if (reserves_.count() > 0)
  
  while (mat->quantity() > batch_size()) {
    Material::Ptr batch = mat->ExtractQty(batch_size());
    reserves_.Push(batch);    
  }
  
  if (mat->quantity() > cyclus::eps_rsrc()) reserves_.Push(mat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::SetUpPhaseNames_() {
  phase_names_.insert(std::make_pair(INITIAL, "initialization"));
  phase_names_.insert(std::make_pair(PROCESS, "processing batch(es)"));
  phase_names_.insert(std::make_pair(WAITING, "waiting for fuel"));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructBatchReactor(cyclus::Context* ctx) {
  return new BatchReactor(ctx);
}

} // namespace cycamore
