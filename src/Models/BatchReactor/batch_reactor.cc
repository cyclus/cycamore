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
      phase_(INITIAL) {
  if (phase_names_.empty()) {
    SetUpPhaseNames_();
  }
  spillover_ = cyclus::Material::CreateBlank(0);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BatchReactor::~BatchReactor() {}
  
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string BatchReactor::schema() {
  return
      "  <!-- cyclus::Material In/Out  -->           \n"
      "  <oneOrMore>                                 \n"
      "  <element name=\"fuel\">                     \n"
      "   <ref name=\"incommodity\"/>                \n"
      "   <ref name=\"inrecipe\"/>                   \n"
      "   <ref name=\"outcommodity\"/>               \n"
      "   <ref name=\"outrecipe\"/>                  \n"
      "  </element>                                  \n"
      "  </oneOrMore>                                \n"
      "                                              \n"
      "  <!-- Facility Parameters -->                \n"
      "  <interleave>                                \n"
      "  <element name=\"processtime\">              \n"
      "    <data type=\"nonNegativeInteger\"/>       \n"
      "  </element>                                  \n"
      "  <element name=\"nbatches\">                 \n"
      "    <data type=\"nonNegativeInteger\"/>       \n"
      "  </element>                                  \n"
      "  <element name =\"batchsize\">               \n"
      "    <data type=\"double\"/>                   \n"
      "  </element>                                  \n"
      "  <optional>                                  \n"
      "    <element name =\"refueltime\">            \n"
      "      <data type=\"nonNegativeInteger\"/>     \n"
      "    </element>                                \n"
      "  </optional>                                 \n"
      "  <optional>                                  \n"
      "    <element name =\"orderlookahead\">        \n"
      "      <data type=\"nonNegativeInteger\"/>     \n"
      "    </element>                                \n"
      "  </optional>                                 \n"
      "  <optional>                                  \n"
      "    <element name =\"norder\">                \n"
      "      <data type=\"nonNegativeInteger\"/>     \n"
      "    </element>                                \n"
      "  </optional>                                 \n"
      "  <optional>                                  \n"
      "    <element name =\"nreload\">               \n"
      "      <data type=\"nonNegativeInteger\"/>     \n"
      "    </element>                                \n"
      "  </optional>                                 \n"
      "  <optional>                                  \n"
      "    <element name =\"initial_condition\">     \n"
      "      <optional>                              \n"
      "        <element name =\"reserves\">          \n"
      "        <element name =\"nbatches\">          \n"
      "          <data type=\"nonNegativeInteger\"/> \n"
      "        </element>                            \n"
      "        <element name =\"commodity\">         \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        <element name =\"recipe\">            \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        </element>                            \n"
      "      </optional>                             \n"
      "      <optional>                              \n"
      "        <element name =\"core\">              \n"
      "        <element name =\"nbatches\">          \n"
      "          <data type=\"nonNegativeInteger\"/> \n"
      "        </element>                            \n"
      "        <element name =\"commodity\">         \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        <element name =\"recipe\">            \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        </element>                            \n"
      "      </optional>                             \n"
      "      <optional>                              \n"
      "        <element name =\"storage\">           \n"
      "        <element name =\"nbatches\">          \n"
      "          <data type=\"nonNegativeInteger\"/> \n"
      "        </element>                            \n"
      "        <element name =\"commodity\">         \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        <element name =\"recipe\">            \n"
      "          <data type=\"string\"/>             \n"
      "        </element>                            \n"
      "        </element>                            \n"
      "      </optional>                             \n"
      "    </element>                                \n"
      "  </optional>                                 \n"
      "                                              \n"
      "  <!-- Recipe Changes  -->                    \n"
      "  <optional>                                  \n"
      "  <oneOrMore>                                 \n"
      "  <element name=\"recipe_change\">            \n"
      "   <element name=\"incommodity\">             \n"
      "     <data type=\"string\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"new_recipe\">              \n"
      "     <data type=\"string\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"time\">                    \n"
      "     <data type=\"nonNegativeInt\"/>          \n"
      "   </element>                                 \n"
      "  </element>                                  \n"
      "  </oneOrMore>                                \n"
      "  </optional>                                 \n"
      "  </interleave>                               \n"
      "                                              \n"
      "  <!-- Power Production  -->                  \n"
      "  <element name=\"commodity_production\">     \n"
      "   <element name=\"commodity\">               \n"
      "     <data type=\"string\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"capacity\">                \n"
      "     <data type=\"double\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"cost\">                    \n"
      "     <data type=\"double\"/>                  \n"
      "   </element>                                 \n"
      "  </element>                                  \n"
      "                                              \n"
      "  <!-- Trade Preferences  -->                 \n"
      "  <optional>                                  \n"
      "  <oneOrMore>                                 \n"
      "  <element name=\"commod_pref\">              \n"
      "   <element name=\"incommodity\">             \n"
      "     <data type=\"string\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"preference\">              \n"
      "     <data type=\"double\"/>                  \n"
      "   </element>                                 \n"
      "  </element>                                  \n"
      "  </oneOrMore>                                \n"
      "  </optional>                                 \n"
      "                                              \n"
      "  <!-- Trade Preference Changes  -->          \n"
      "  <optional>                                  \n"
      "  <oneOrMore>                                 \n"
      "  <element name=\"pref_change\">              \n"
      "   <element name=\"incommodity\">             \n"
      "     <data type=\"string\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"new_pref\">                \n"
      "     <data type=\"double\"/>                  \n"
      "   </element>                                 \n"
      "   <element name=\"time\">                    \n"
      "     <data type=\"nonNegativeInt\"/>          \n"
      "   </element>                                 \n"
      "  </element>                                  \n"
      "  </oneOrMore>                                \n"
      "  </optional>                                 \n";
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::InitModuleMembers(cyclus::QueryEngine* qe) {  
  using boost::lexical_cast;
  using cyclus::Commodity;
  using cyclus::CommodityProducer;
  using cyclus::GetOptionalQuery;
  using cyclus::QueryEngine;
  using std::string;
  
  // in/out fuel
  int nfuel = qe->NElementsMatchingQuery("initial_condition");
  for (int i = 0; i < nfuel, i++) {
    QueryEngine* fuel = qe->QueryElement("fuel", i);
    crctx_.AddInCommod(
        fuel->GetElementContent("incommodity"),
        fuel->GetElementContent("inrecipe"),
        fuel->GetElementContent("outcommodity"),
        fuel->GetElementContent("outrecipe"));
  }

  // facility data required
  string data;
  data = qe->GetElementContent("processtime");
  process_time(lexical_cast<int>(data));
  data = qe->GetElementContent("nbatches");
  n_batches(lexical_cast<int>(data));
  data = qe->GetElementContent("batchsize");
  batch_size(lexical_cast<double>(data));

  // facility data optional
  int time;
  time = GetOptionalQuery<int>(qe, "refueltime", refuel_time());
  refuel_time(time);
  time = GetOptionalQuery<int>(qe, "orderlookahead", preorder_time());
  preorder_time(time);

  int n;
  n= GetOptionalQuery<int>(qe, "nreload", n_load());
  n_load(n);
  n = GetOptionalQuery<int>(qe, "norder", n_load());
  n_reserves(n);

  // initial condition
  if (qe->NElementsMatchingQuery("initial_condition") > 0) {
    QueryEngine* ic = qe->QueryElement("initial_condition");
    if (ic->NElementsMatchingQuery("reserves") > 0) {
      QueryEngine* reserves = ic->QueryElement("reserves");
      ics_.AddReserves(
          lexical_cast<int>(ic->GetElementContent("nbatches")),
          ic->GetElementContent("recipe"),
          ic->GetElementContent("commodity"));
    }
    if (ic->NElementsMatchingQuery("core") > 0) {
      QueryEngine* core = ic->QueryElement("core");
      ics_.AddCore(
          lexical_cast<int>(ic->GetElementContent("nbatches")),
          ic->GetElementContent("recipe"),
          ic->GetElementContent("commodity"));
    }
    if (ic->NElementsMatchingQuery("storage") > 0) {
      QueryEngine* storage = ic->QueryElement("storage");
      ics_.AddStorage(
          lexical_cast<int>(ic->GetElementContent("nbatches")),
          ic->GetElementContent("recipe"),
          ic->GetElementContent("commodity"));
    }
  }
      
  // commodity production
  QueryEngine* commodity = qe->QueryElement("commodity_production");
  Commodity commod(commodity->GetElementContent("commodity"));
  AddCommodity(commod);
  data = commodity->GetElementContent("capacity");
  CommodityProducer::SetCapacity(commod, lexical_cast<double>(data));
  data = commodity->GetElementContent("cost");
  CommodityProducer::SetCost(commod, lexical_cast<double>(data));

  // trade preferences
  int nprefs = qe->NElementsMatchingQuery("commod_pref");
  std::string c;
  double pref;
  if (nprefs > 0) {
    for (int i = 0; i < nprefs; i++) {
      QueryEngine* cp = qe->QueryElement("commod_pref", i);
      c = cp->GetElementContent("incommodity");
      pref = lexical_cast<double>(cp->GetElementContent("preference"));
      commod_prefs_[c] = pref;
    }
  }

  // pref changes
  int nchanges = qe->NElementsMatchingQuery("pref_changes");
  if (nchanges > 0) {
    for (int i = 0; i < nchanges; i++) {
      QueryEngine* cp = qe->QueryElement("pref_changes", i);
      c = cp->GetElementContent("incommodity");
      pref = lexical_cast<double>(cp->GetElementContent("new_pref"));
      time = lexical_cast<int>(cp->GetElementContent("time"));
      pref_changes_[time].push_back(std::make_pair(c, pref));
    }
  }
  
  // recipe changes
  std::string rec;
  int nchanges = qe->NElementsMatchingQuery("recipe_change");
  if (nchanges > 0) {
    for (int i = 0; i < nchanges; i++) {
      QueryEngine* cp = qe->QueryElement("recipe_change", i);
      c = cp->GetElementContent("incommodity");
      rec = cp->GetElementContent("new_recipe");
      time = lexical_cast<int>(cp->GetElementContent("time"));
      recipe_changes_[time].push_back(std::make_pair(c, rec));
    }
  }  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* BatchReactor::Clone() {
  BatchReactor* m = new BatchReactor(context());
  m->InitFrom(this);
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::InitFrom(BatchReactor* m) {
  // in/out
  in_commodity(m->in_commodity());
  out_commodity(m->out_commodity());
  in_recipe(m->in_recipe());
  out_recipe(m->out_recipe());

  // facility params
  process_time(m->process_time());
  preorder_time(m->preorder_time());
  refuel_time(m->refuel_time());
  n_batches(m->n_batches());
  n_load(m->n_load());
  n_reserves(m->n_reserves());
  batch_size(m->batch_size());

  // commodity production
  CopyProducedCommoditiesFrom(m);

  // ics
  ics(m->ics());
  
  // trade preferences
  commod_prefs(m->commod_prefs());
  pref_changes_ = m->pref_changes_;

  // recipe changes
  recipe_changes_ = m->recipe_changes_;

  crctx_ = m->crctx_;
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
  using cyclus::Material;

  FacilityModel::Deploy(parent);
  phase(INITIAL);
  spillover_ = Material::Create(this, 0.0, context()->GetRecipe(in_recipe_));

  Material::Ptr mat;
  if (ics_.reserves) {
    for (int i = 0; i < ics_.n_reserves; ++i) {
      mat = Material::Create(this,
                             batch_size(),
                             context()->GetRecipe(ics_.reserves_rec));
      crctx_.AddMat(ics_.reserves_commod, mat);
      reserves_.Push(mat);
    }
  }
  if (ics_.core) {
    for (int i = 0; i < ics_.n_core; ++i) {
      mat = Material::Create(this,
                             batch_size(),
                             context()->GetRecipe(ics_.core_rec));
      crctx_.AddMat(ics_.core_commod, mat);
      core_.Push(mat);
    }
  }
  if (ics_.storage) {
    for (int i = 0; i < ics_.n_storage; ++i) {
      mat = Material::Create(this,
                             batch_size(),
                             context()->GetRecipe(ics_.storage_rec));
      crctx_.AddMat(ics_.storage_commod, mat);
      storage_[storage_commod].Push(mat);
    }
  }

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

  // change preferences if its time
  if (pref_changes_.count(time)) {
    std::vector< std::pair< std::string, double> >&
        changes = pref_changes_[time];
    for (int i = 0; i < changes.size(); i++) {
      commod_prefs_[changes[i].first] = changes[i].second;
    }
  }

  // change recipes if its time
  if (recipe_changes_.count(time)) {
    std::vector< std::pair< std::string, std::string> >&
        changes = recipe_changes_[time];
    for (int i = 0; i < changes.size(); i++) {
      crctx_.UpdateInRec(changes[i].first, changes[i].second);
    }
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
                   - core_.quantity() - reserves_.quantity()
                   - spillover_->quantity();
      if (preorder_time() == 0) order_size += batch_size() * n_reserves();
      if (order_size > 0) {
        RequestPortfolio<Material>::Ptr p = GetOrder_(order_size);
        set.insert(p);
      }
      break;

    // the default case is to request the reserve amount if the order time has
    // been reached
    default:
      order_size = n_reserves() * batch_size()
                   - reserves_.quantity() - spillover_->quantity();
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
  using cyclus::Material;
  
  // std::map<std::string, Material::Ptr> mat_commods;
   
  // std::vector< std::pair<cyclus::Trade<cyclus::Material>,
  //                        cyclus::Material::Ptr> >::const_iterator trade;
  
  // for (trade = responses.begin(); trade != responses.end(); ++trade) {
  // }
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
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  const std::vector<std::string>& commods = crctx_.out_commods();
  std::vector<std::string>::const_iterator it;
  for (it = commods.begin(); it != commods.end(); ++it) {
    BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                                *it,
                                                &storage_[*it]);
    if (!port->bids().empty()) ports.insert(port);
  }
  
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    LOG(cyclus::LEV_INFO5, "BReact") << name() << " just received an order.";

    std::string commodity = it->request->commodity();
    double qty = it->amt;
    Material::Ptr response = TradeResponse_(qty, &storage_[commodity]);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "BatchReactor") << name()
                                           << " just received an order"
                                           << " for " << qty
                                           << " of " << commodity;
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
  while(n_core() < n_batches() && reserves_.count() > 0) {
    MoveBatchIn_();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::MoveBatchIn_() {
  LOG(cyclus::LEV_DEBUG2, "BReact") << "BatchReactor " << name() << " added"
                                    <<  " a batch from its core.";
  try {
    core_.Push(reserves_.Pop());
  } catch(cyclus::Error& e) {
      e.msg(Model::InformErrorMsg(e.msg()));
      throw e;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::MoveBatchOut_() {
  using cyclus::Material;
  using cyclus::ResCast;
  
  LOG(cyclus::LEV_DEBUG2, "BReact") << "BatchReactor " << name() << " removed"
                                    <<  " a batch from its core.";
  try {
    Material::Ptr mat = ResCast<Material>(core_.Pop());
    std::string incommod = crctx_.commod(mat);
    std::string outcommod = crctx_.out_commodity(incommod);
    std::string outrecipe = crctx_.out_recipe(crctx_.in_recipe(incommod));
    mat->Transmute(context()->GetRecipe(outrecipe));
    crctx_.UpdateMat(outcommod, mat);
    storage_[outcommod].Push(mat);
  } catch(cyclus::Error& e) {
      e.msg(Model::InformErrorMsg(e.msg()));
      throw e;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::RequestPortfolio<cyclus::Material>::Ptr
BatchReactor::GetOrder_(double size) {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  
  const std::vector<std::string>& commods = crctx_.in_commods();
  std::vector<std::string>::const_iterator it;
  std::string recipe;
  Material::Ptr mat;
  for (it = commods.begin(); it != commods.end(); ++it) {
    recipe = crctx_.in_recipe(*it);
    mat =
        Material::CreateUntracked(size, context()->GetRecipe(recipe));
    port->AddRequest(mat, this, *it, commod_prefs_[*it]);
  }

  CapacityConstraint<Material> cc(size);
  port->AddConstraint(cc);

  LOG(cyclus::LEV_DEBUG3, "BReact") << "BatchReactor " << name()
                                    << " is making an order:";
  LOG(cyclus::LEV_DEBUG3, "BReact") << "          size: " << size;
  LOG(cyclus::LEV_DEBUG3, "BReact") << "     commodity: " << in_commodity_;
  LOG(cyclus::LEV_DEBUG3, "BReact") << "    preference: "
                                    << commod_prefs_[in_commodity_];

  return port;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BatchReactor::AddBatches_(cyclus::Material::Ptr mat) {
  using cyclus::Material;
  using cyclus::ResCast;

  LOG(cyclus::LEV_DEBUG3, "BReact") << "BatchReactor " << name()
                                    << " is adding " << mat->quantity()
                                    << " of material to its reserves.";

  spillover_->Absorb(mat);
  
  while (spillover_->quantity() >= batch_size()) {
    Material::Ptr batch = spillover_->ExtractQty(batch_size());
    reserves_.Push(batch);    
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr BatchReactor::GetBids_(
    const cyclus::CommodMap<cyclus::Material>::type& commod_requests,
    std::string commod,
    cyclus::ResourceBuff* buffer) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
    
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
  if (commod_requests.count(commod) > 0 && buffer->quantity() > 0) {
    const std::vector<Request<Material>::Ptr>& requests =
        commod_requests.at(commod);
    
    std::vector<Request<Material>::Ptr>::const_iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      const Request<Material>::Ptr req = *it;
      double qty = std::min(req->target()->quantity(), buffer->quantity());
      Material::Ptr offer =
          Material::CreateUntracked(qty, context()->GetRecipe(out_recipe_));
      port->AddBid(req, offer, this);
    }
    
    CapacityConstraint<Material> cc(buffer->quantity());
    port->AddConstraint(cc);
  }

  return port;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr BatchReactor::TradeResponse_(
    double qty,
    cyclus::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from inventory and blob it into one material
    manifest = ResCast<Material>(buffer->PopQty(qty));  
  } catch(cyclus::Error& e) {
    e.msg(Model::InformErrorMsg(e.msg()));
    throw e;
  }
  
  Material::Ptr response = manifest[0];
  crctx_.RemoveRsrc(response);
  for (int i = 1; i < manifest.size(); i++) {
    crctx_.RemoveRsrc(manifest[i]);
    response->Absorb(manifest[i]);
  }
  return response;
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
