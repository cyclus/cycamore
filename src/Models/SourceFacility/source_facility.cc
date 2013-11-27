// source_facility.cc
// Implements the SourceFacility class
#include "source_facility.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "capacity_constraint.h"
#include "cyc_limits.h"
#include "context.h"
#include "error.h"
#include "exchange_context.h"
#include "logger.h"
#include "request.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility(cyclus::Context* ctx)
  : cyclus::FacilityModel(ctx),
    cyclus::Model(ctx),
    out_commod_(""),
    recipe_name_(""),
    commod_price_(0),
    capacity_(std::numeric_limits<double>::max()) { }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::~SourceFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::schema() {
  return
    "  <element name =\"output\">          \n"
    "    <ref name=\"outcommodity\"/>      \n"
    "    <optional>                        \n"
    "      <ref name=\"output_capacity\"/> \n"
    "    </optional>                       \n"
    "    <element name=\"recipe\">         \n"
    "      <data type=\"string\"/>         \n"
    "    </element>                        \n"
    "  </element>                          \n";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  using std::numeric_limits;
  using boost::lexical_cast;
  cyclus::QueryEngine* output = qe->QueryElement("output");

  set_recipe(output->GetElementContent("recipe"));

  string data = output->GetElementContent("outcommodity");
  set_commodity(data);
  cyclus::Commodity commod(data);
  cyclus::CommodityProducer::AddCommodity(commod);

  double capacity =
    cyclus::GetOptionalQuery<double>(output,
                                     "output_capacity",
                                     numeric_limits<double>::max());
  cyclus::CommodityProducer::SetCapacity(commod, capacity);
  set_capacity(capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '"
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step ";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacility::Clone() {
  SourceFacility* m = new SourceFacility(*this);
  m->InitFrom(this);

  m->set_commodity(commodity());
  m->set_capacity(capacity());
  m->set_recipe(recipe());
  m->CopyProducedCommoditiesFrom(this);

  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is ticking {";
  LOG(cyclus::LEV_INFO4, "SrcFac") << "will offer " << capacity_
                                   << " kg of "
                                   << out_commod_ << ".";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr SourceFacility::__GetOffer(
    const cyclus::Material::Ptr target) const {
  using cyclus::Material;
  double qty = std::min(target->quantity(), capacity_);
  return Material::CreateUntracked(qty, context()->GetRecipe(recipe_name_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
SourceFacility::AddMatlBids(cyclus::ExchangeContext<cyclus::Material>* ec) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::Request;
  
  std::set<BidPortfolio<Material>::Ptr> ports;
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
  const std::vector<Request<Material>::Ptr>& requests =
      ec->RequestsForCommod(out_commod_);

  std::vector<Request<Material>::Ptr>::const_iterator it;
  for (it = requests.begin(); it != requests.end(); ++it) {
    const Request<Material>::Ptr req = *it;
    Material::Ptr offer = __GetOffer(req->target());
    Bid<Material>::Ptr bid(new Bid<Material>(req, offer, this));
    port->AddBid(bid);
  }

  CapacityConstraint<Material> cc(capacity_);
  port->AddConstraint(cc);
  ports.insert(port);
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::PopulateMatlTradeResponses(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::StateError;
  using cyclus::Trade;

  double total = 0;
  std::vector< cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->request->target()->quantity();
    total += qty;
    if (total > capacity_) {
      throw StateError("SourceFac " + name()
                       + " is being asked to provide more than its capacity.");
    }
    Material::Ptr response =
        Material::Create(this, qty, context()->GetRecipe(recipe_name_));
    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "SrcFac") << name() << " just received an order"
                                     << " for " << qty
                                     << " of " << out_commod_;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructSourceFacility(cyclus::Context* ctx) {
  return new SourceFacility(ctx);
}
} // namespace cycamore
