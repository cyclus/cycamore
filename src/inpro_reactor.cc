// inpro_reactor.cc
// Implements the InproReactor class
#include <sstream>
#include <cmath>

#include <boost/lexical_cast.hpp>

#include "cyc_limits.h"
#include "context.h"
#include "error.h"
#include "logger.h"
#include "generic_resource.h"

#include "inpro_reactor.h"

namespace cycamore {

// static members
std::map<Phase, std::string> InproReactor::phase_names_ = \
  std::map<Phase, std::string>();

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
InproReactor::InproReactor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      cycle_length_(1),
      refuel_delay_(0),
      batches_per_core_(1),
      in_core_loading_(1),
      out_core_loading_(1),
      in_commodity_(""),
      in_recipe_(""),
      out_commodity_(""),
      out_recipe_(""),
      cycle_timer_(0),
      phase_(INIT) {
  preCore_.set_capacity(cyclus::kBuffInfinity);
  inCore_.set_capacity(cyclus::kBuffInfinity);
  postCore_.set_capacity(cyclus::kBuffInfinity);
  if (phase_names_.size() < 1) {
    SetUpPhaseNames();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
InproReactor::~InproReactor() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::schema() {
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
    "  <element name=\"cyclelength\">            \n"
    "    <data type=\"nonNegativeInteger\"/>     \n"
    "  </element>                                \n"
    "  <optional>                                \n"
    "    <element name =\"refueldelay\">         \n"
    "      <data type=\"nonNegativeInteger\"/>   \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "  <element name =\"incoreloading\">         \n"
    "    <data type=\"double\"/>                 \n"
    "  </element>                                \n"
    "  <optional>                                \n"
    "    <element name =\"outcoreloading\">      \n"
    "      <data type=\"double\"/>               \n"
    "    </element>                              \n"
    "  </optional>                               \n"
    "  <element name=\"batchespercore\">         \n"
    "    <data type=\"nonNegativeInteger\"/>     \n"
    "  </element>                                \n"
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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::InitFrom(cyclus::InfileTree* qe) {
  cyclus::Facility::InitFrom(qe);
  qe = qe->SubTree("agent/*");

  using std::string;
  using boost::lexical_cast;
  cyclus::InfileTree* input = qe->SubTree("fuel_input");
  set_in_commodity(input->GetString("incommodity"));
  set_in_recipe(input->GetString("inrecipe"));

  cyclus::InfileTree* output = qe->SubTree("fuel_output");
  set_out_commodity(output->GetString("outcommodity"));
  set_out_recipe(output->GetString("outrecipe"));

  string data;
  data = qe->GetString("cyclelength");
  set_cycle_length(lexical_cast<int>(data));

  int delay =
      cyclus::OptionalQuery<int>(qe, "refueldelay", refuel_delay());
  set_refuel_delay(delay);

  data = qe->GetString("incoreloading");
  set_in_core_loading(lexical_cast<double>(data));

  double loading = 
      cyclus::OptionalQuery<double>(qe, "outcoreloading", in_core_loading());
  set_out_core_loading(loading);

  data = qe->GetString("batchespercore");
  set_batches_per_core(lexical_cast<int>(data));

  cyclus::InfileTree* commodity = qe->SubTree("commodity_production");
  cyclus::Commodity commod(commodity->GetString("commodity"));
  AddCommodity(commod);
  data = commodity->GetString("capacity");
  cyclus::CommodityProducer::SetCapacity(commod,
                                                        lexical_cast<double>(data));
  data = commodity->GetString("cost");
  cyclus::CommodityProducer::SetCost(commod,
                                                    lexical_cast<double>(data));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::str() {
  std::stringstream ss;
  ss << cyclus::Facility::str();
  ss << " has facility parameters {"
     << ", Cycle Length = " << cycle_length()
     << ", Refuel Delay = " << refuel_delay()
     << ", InCore Loading = " << in_core_loading()
     << ", OutCore Loading = " << out_core_loading()
     << ", Batches Per Core = " << batches_per_core()
     << ", converts commodity '" << in_commodity()
     << "' into commodity '" << out_commodity()
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Agent* InproReactor::Clone() {
  InproReactor* m = new InproReactor(context());
  m->InitFrom(this);
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::InitFrom(InproReactor* m) {
  cyclus::Facility::InitFrom(m);
  set_cycle_length(m->cycle_length());
  set_refuel_delay(m->refuel_delay());
  set_in_core_loading(m->in_core_loading());
  set_out_core_loading(m->out_core_loading());
  set_batches_per_core(m->batches_per_core());
  set_in_commodity(m->in_commodity());
  set_out_commodity(m->out_commodity());
  set_in_recipe(m->in_recipe());
  set_out_recipe(m->out_recipe());
  CopyProducedCommoditiesFrom(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::Build(cyclus::Agent* parent) {
  Facility::Build(parent);
  preCore_.set_capacity(in_core_loading());
  inCore_.set_capacity(in_core_loading());
  reset_cycle_timer();
  SetPhase(BEGIN);
  LOG(cyclus::LEV_DEBUG2, "BReact") << "Batch Reactor " << prototype()
                                    << " is entering the simuluation with members:";
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * in core loading: " <<
                                    in_core_loading();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * out core loading: " <<
                                    out_core_loading();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * pre core capacity: " <<
                                    preCore_.capacity();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * in core capacity: " <<
                                    inCore_.capacity();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * cycle timer: " << cycle_timer_;
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * phase: " << phase_names_[phase_];
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::Tick(int time) {
  LOG(cyclus::LEV_INFO3, "BReact") << prototype() << " is ticking at time "
                                   << time << " {";
  LOG(cyclus::LEV_DEBUG3, "BReact") << "The current phase is: "
                                    << phase_names_[phase_];


  if (lifetime() != -1 && time >= lifetime() + enter_time()) {
    SetPhase(END);
  }

  switch (phase()) {
    case INIT:
      // intentional fall through

    case OPERATION:
      cycle_timer_++;
      break;

    case REFUEL:
      OffloadBatch();

    case REFUEL_DELAY:
      // intentional fall through

    case WAITING:
      // intentional fall through

    case BEGIN:
      cycle_timer_++;
      break;

    case END:
      OffloadCore();
      break;

    default:
      std::string msg =
          "InproReactors have undefined behvaior during ticks for phase: "
          + phase_names_[phase_];
      throw cyclus::Error(msg);
      break;
  }

  LOG(cyclus::LEV_INFO3, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
InproReactor::GetMatlRequests() {
  std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> set;
  double fuel_quantity = preCore_.quantity() + inCore_.quantity();
  double request = in_core_loading() - fuel_quantity;
  
  if (request > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "BReact") << " making requests {";
    LOG(cyclus::LEV_INFO5, "BReact") << prototype() << " is requesting " << request
                                     << " kg of " << in_commodity_ << ".";
    LOG(cyclus::LEV_INFO4, "BReact") << "}";
  
    set.insert(GetOrder_(request));
  }
  return set;
}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
InproReactor::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;
  BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                              out_commodity_,
                                              &postCore_);
  if (!port->bids().empty()) {
    LOG(cyclus::LEV_INFO4, "BReact") << " making offers {";
    LOG(cyclus::LEV_INFO5, "BReact") << prototype() << " is offering "
                                     << postCore_.quantity()
                                     << " kg of " << out_commodity_ << ".";
    LOG(cyclus::LEV_INFO4, "BReact") << "}";

    ports.insert(port);
  }
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  
  std::map<std::string, Material::Ptr> mat_commods;
   
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator trade;

  double preQuantity = preCore_.quantity();
  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    preCore_.Push(trade->second);
  }  
  double added = preCore_.quantity() - preQuantity;
  LOG(cyclus::LEV_DEBUG2, "BReact") << "InproReactor " << prototype() << " added "
                                    << added << " to its precore buffer.";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    LOG(cyclus::LEV_INFO5, "BReact") << prototype() << " just received an order.";

    double qty = it->amt;
    Material::Ptr response = TradeResponse_(qty, &postCore_);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "InproReactor") << prototype()
                                           << " just received an order"
                                           << " for " << qty
                                           << " of " << out_commodity_;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::Tock(int time) {
  using std::string;
  using boost::lexical_cast;
  LOG(cyclus::LEV_INFO3, "BReact") << prototype() << " is tocking {";
  LOG(cyclus::LEV_DEBUG3, "BReact") << "The current phase is: "
                                    << phase_names_[phase_];

  string msg;

  switch (phase()) {

    case END:
      // if ( postCore_.empty() )
      //   Decommission();
      break;

    case BEGIN:
      // intentional fall through

    case WAITING:
      LoadCore();
      if (CoreFilled()) {

        SetPhase(OPERATION);
        reset_cycle_timer();
      } else {
        SetPhase(WAITING);
      }
      break;

    case REFUEL:
      SetPhase(REFUEL_DELAY);
      time_delayed_ = 0;
    case REFUEL_DELAY:
      LoadCore();
      if (time_delayed_ > refuel_delay() && CoreFilled()) {
        SetPhase(OPERATION);
        reset_cycle_timer();
      } else {
        ++time_delayed_;
      }
      break;

    case OPERATION:
      if (CycleComplete()) {
        SetPhase(REFUEL);
      }
      break;

    default:
      msg = "InproReactors have undefined behvaior during tocks for phase: "
            + phase_names_[phase_];
      throw cyclus::Error(msg);
      break;
  }

  LOG(cyclus::LEV_DEBUG3, "BReact") << "cycle timer: "
                                    << cycle_timer_;
  LOG(cyclus::LEV_DEBUG3, "BReact") << "delay: "
                                    << time_delayed_;
  LOG(cyclus::LEV_INFO3, "BReact") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_cycle_length(int time) {
  cycle_length_ = time;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int InproReactor::cycle_length() {
  return cycle_length_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_refuel_delay(int time) {
  refuel_delay_ = time;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int InproReactor::refuel_delay() {
  return refuel_delay_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_in_core_loading(double size) {
  in_core_loading_ = size;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double InproReactor::in_core_loading() {
  return in_core_loading_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_out_core_loading(double size) {
  out_core_loading_ = size;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double InproReactor::out_core_loading() {
  return out_core_loading_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_batches_per_core(int n) {
  batches_per_core_ = n;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int InproReactor::batches_per_core() {
  return batches_per_core_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double InproReactor::BatchLoading() {
  return in_core_loading_ / batches_per_core_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_in_commodity(std::string name) {
  in_commodity_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::in_commodity() {
  return in_commodity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_in_recipe(std::string name) {
  in_recipe_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::in_recipe() {
  return in_recipe_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_out_commodity(std::string name) {
  out_commodity_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::out_commodity() {
  return out_commodity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::set_out_recipe(std::string name) {
  out_recipe_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string InproReactor::out_recipe() {
  return out_recipe_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Phase InproReactor::phase() {
  return phase_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool InproReactor::CheckDecommissionCondition() {
  bool empty = (preCore_.empty() && inCore_.empty() &&
                postCore_.empty());
  // if (!empty) {
  //   string msg = "Can't delete a InproReactor with material still in its inventory.";
  //   throw cyclus::CycInproReactorDestructException(msg);
  // }
  return empty;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::SetPhase(Phase p) {
  LOG(cyclus::LEV_DEBUG2, "BReact") << "InproReactor " << prototype()
                                    << " is changing phases -";
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * from phase: " << phase_names_[phase_];
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * to phase: " << phase_names_[p];
  phase_ = p;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::SetUpPhaseNames() {
  using std::make_pair;
  phase_names_.insert(make_pair(INIT, "initialization"));
  phase_names_.insert(make_pair(BEGIN, "beginning"));
  phase_names_.insert(make_pair(OPERATION, "operation"));
  phase_names_.insert(make_pair(REFUEL, "refueling"));
  phase_names_.insert(make_pair(REFUEL_DELAY, "refueling with delay"));
  phase_names_.insert(make_pair(WAITING, "waiting for fuel"));
  phase_names_.insert(make_pair(END, "ending"));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::reset_cycle_timer() {
  cycle_timer_ = 1;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool InproReactor::CycleComplete() {
  return (cycle_timer_ >= cycle_length_ - 1);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool InproReactor::CoreFilled() {
  LOG(cyclus::LEV_DEBUG2, "BReact") << "Querying whether the core is filled -";
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * quantity in core: " <<
                                    inCore_.quantity();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  * core capacity: " <<
                                    inCore_.capacity();
  // @MJGFlag need to assert that the in core capacity must be > 0
  // 9/29/12 error with a negative in core capacity
  return (abs(inCore_.quantity() - inCore_.capacity()) < cyclus::eps());
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RequestPortfolio<cyclus::Material>::Ptr
InproReactor::GetOrder_(double size) {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat = Material::CreateUntracked(
      size,
      context()->GetRecipe(in_recipe_));
  port->AddRequest(mat, this, in_commodity_);
  
  LOG(cyclus::LEV_DEBUG3, "IReact") << "InproReactor " << prototype()
                                    << " is making an order:";
  LOG(cyclus::LEV_DEBUG3, "IReact") << "          size: " << size;
  LOG(cyclus::LEV_DEBUG3, "IReact") << "     commodity: "
                                    << in_commodity_;

  CapacityConstraint<Material> cc(size);
  port->AddConstraint(cc);

  return port;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr InproReactor::GetBids_(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests,
    std::string commod,
    cyclus::ResourceBuff* buffer) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Composition;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ResCast;
  using cyclus::ResourceBuff;
    
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
  if (commod_requests.count(commod) > 0 && buffer->quantity() > 0) {
    std::vector<Request<Material>*>& requests = commod_requests[commod];

    // get offer composition
    Material::Ptr back = ResCast<Material>(buffer->Pop(ResourceBuff::BACK));
    Composition::Ptr comp = back->comp();
    buffer->Push(back);
    
    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      double qty = std::min(req->target()->quantity(), buffer->quantity());
      Material::Ptr offer =
          Material::CreateUntracked(qty, comp);
      port->AddBid(req, offer, this);
    }
    
    CapacityConstraint<Material> cc(buffer->quantity());
    port->AddConstraint(cc);
  }

  return port;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr InproReactor::TradeResponse_(
    double qty,
    cyclus::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from inventory and blob it into one material
    manifest = ResCast<Material>(buffer->PopQty(qty));  
  } catch(cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }
  
  Material::Ptr response = manifest[0];
  for (int i = 1; i < manifest.size(); i++) {
    response->Absorb(manifest[i]);
  }
  return response;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::OffloadBatch() {
  using cyclus::Context;
  using cyclus::Material;
  
  Material::Ptr m = cyclus::ResCast<Material>(inCore_.Pop());
  double factor = out_core_loading() / in_core_loading();
  double loss = m->quantity() * (1 - factor);

  m->ExtractQty(loss); // mass discrepancy
  m->Transmute(context()->GetRecipe(out_recipe()));
  postCore_.Push(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::LoadCore() {
  using cyclus::Material;
  if (preCore_.quantity() >= BatchLoading()) {
    // combine materials into a single resource of batch size and move to inCore buf
    std::vector<Material::Ptr> mats = cyclus::ResCast<Material>(preCore_.PopQty(BatchLoading()));
    Material::Ptr m = mats[0];
    for (int i = 1; i < mats.size(); ++i) {
      m->Absorb(mats[i]);
    }
    inCore_.Push(m);
    LOG(cyclus::LEV_DEBUG2, "BReact") << "InproReactor " << prototype()
                                      << " moved fuel into the core:";
    LOG(cyclus::LEV_DEBUG2, "BReact") << "  precore level: " << preCore_.quantity();
    LOG(cyclus::LEV_DEBUG2, "BReact") << "  incore level: " << inCore_.quantity();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void InproReactor::OffloadCore() {
  while (!inCore_.empty()) {
    OffloadBatch();
  }
  LOG(cyclus::LEV_DEBUG2, "BReact") << "InproReactor " << prototype()
                                    << " removed a core of fuel from the core:";
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  precore level: " << preCore_.quantity();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  incore level: " << inCore_.quantity();
  LOG(cyclus::LEV_DEBUG2, "BReact") << "  postcore level: " <<
                                    postCore_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructInproReactor(cyclus::Context* ctx) {
  return new InproReactor(ctx);
}
} // namespace cycamore
