#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "source_facility.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility(cyc::Context* ctx)
  : cyc::FacilityModel(ctx),
    out_commod_(""),
    recipe_name_(""),
    capacity_(std::numeric_limits<double>::max()) {}

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


void SourceFacility::InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di) {
  cyc::FacilityModel::InfileToDb(qe, di);
  qe = qe->QueryElement("model/" + model_impl());
  
  using std::numeric_limits;
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

void SourceFacility::InitFrom(cyc::QueryBackend* b) {
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

void SourceFacility::Snapshot(cyc::DbInit di) {
  cyc::FacilityModel::Snapshot(di);
  di.NewDatum("Info")
    ->AddVal("recipe", recipe_name_)
    ->AddVal("out_commod", out_commod_)
    ->AddVal("capacity", capacity_)
    ->AddVal("curr_capacity", current_capacity_)
    ->Record();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyc::FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '"
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step ";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Model* SourceFacility::Clone() {
  SourceFacility* m = new SourceFacility(context());
  m->InitFrom(this);
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::InitFrom(SourceFacility* m) {
  FacilityModel::InitFrom(m);
  commodity(m->commodity());
  capacity(m->capacity());
  recipe(m->recipe());
  CopyProducedCommoditiesFrom(m);
  current_capacity_ = capacity();  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::Tick(int time) {
  LOG(cyc::LEV_INFO3, "SrcFac") << prototype() << " is ticking {";
  LOG(cyc::LEV_INFO4, "SrcFac") << "will offer " << capacity_
                                   << " kg of "
                                   << out_commod_ << ".";
  LOG(cyc::LEV_INFO3, "SrcFac") << "}";
  current_capacity_ = capacity_; // reset capacity
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::Tock(int time) {
  LOG(cyc::LEV_INFO3, "SrcFac") << prototype() << " is tocking {";
  LOG(cyc::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Material::Ptr SourceFacility::GetOffer(
    const cyc::Material::Ptr target) const {
  using cyc::Material;
  double qty = std::min(target->quantity(), capacity_);
  return Material::CreateUntracked(qty, context()->GetRecipe(recipe_name_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
SourceFacility::GetMatlBids(
    const cyc::CommodMap<cyc::Material>::type& commod_requests) {
  using cyc::Bid;
  using cyc::BidPortfolio;
  using cyc::CapacityConstraint;
  using cyc::Material;
  using cyc::Request;
  
  std::set<BidPortfolio<Material>::Ptr> ports;
  
  if (commod_requests.count(out_commod_) > 0) {
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
    const std::vector<Request<Material>::Ptr>& requests = commod_requests.at(
        out_commod_);

    std::vector<Request<Material>::Ptr>::const_iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      const Request<Material>::Ptr req = *it;
      Material::Ptr offer = GetOffer(req->target());
      port->AddBid(req, offer, this);
    }

    CapacityConstraint<Material> cc(capacity_);
    port->AddConstraint(cc);
    ports.insert(port);
  }
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::GetMatlTrades(
    const std::vector< cyc::Trade<cyc::Material> >& trades,
    std::vector<std::pair<cyc::Trade<cyc::Material>,
                          cyc::Material::Ptr> >& responses) {
  using cyc::Material;
  using cyc::Trade;

  double provided = 0;
  std::vector< cyc::Trade<cyc::Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    double qty = it->amt;
    current_capacity_ -= qty;
    provided += qty;
    // @TODO we need a policy on negatives..
    Material::Ptr response = Material::Create(this,
                                              qty,
                                              context()->GetRecipe(recipe_name_));
    responses.push_back(std::make_pair(*it, response));
    LOG(cyc::LEV_INFO5, "SrcFac") << prototype() << " just received an order"
                                     << " for " << qty
                                     << " of " << out_commod_;
  }
  if (cyc::IsNegative(current_capacity_)) {
    std::stringstream ss;
    ss << "is being asked to provide " << provided
       << " but its capacity is " << capacity_ << "."; 
    throw cyc::ValueError(Model::InformErrorMsg(ss.str()));
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyc::Model* ConstructSourceFacility(cyc::Context* ctx) {
  return new SourceFacility(ctx);
}

} // namespace cycamore
