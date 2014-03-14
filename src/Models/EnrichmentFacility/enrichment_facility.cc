// enrichment_facility.cc
// Implements the EnrichmentFacility class
#include "enrichment_facility.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include <boost/lexical_cast.hpp>

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::EnrichmentFacility(cyc::Context* ctx)
  : cyc::FacilityModel(ctx),
    tails_assay_(0),
    feed_assay_(0),
    swu_capacity_(0),
    initial_reserves_(0),
    in_commod_(""),
    in_recipe_(""),
    out_commod_("") {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::~EnrichmentFacility() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string EnrichmentFacility::schema() {
  return
    "  <element name =\"input\">                  \n"
    "    <ref name=\"incommodity\"/>              \n"
    "    <ref name=\"inrecipe\"/>                 \n"
    "    <optional>                               \n"
    "      <ref name=\"inventorysize\"/>          \n"
    "    </optional>                              \n"
    "  </element>                                 \n"
    "  <element name =\"output\">                 \n"
    "    <ref name=\"outcommodity\"/>             \n"
    "     <element name =\"tails_assay\">         \n"
    "       <data type=\"double\"/>               \n"
    "     </element>                              \n"
    "    <optional>                               \n"
    "      <element name =\"swu_capacity\">       \n"
    "        <data type=\"double\"/>              \n"
    "      </element>                             \n"
    "    </optional>                              \n"
    "  </element>                                 \n"
    "  <optional>                                 \n"
    "    <element name =\"initial_condition\">    \n"
    "       <element name =\"reserves_qty\">      \n"
    "         <data type=\"double\"/>             \n"
    "       </element>                            \n"
    "    </element>                               \n"
    "  </optional>                                \n";
}

void EnrichmentFacility::InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di) {
  cyc::FacilityModel::InfileToDb(qe, di);
  qe = qe->QueryElement("model/" + model_impl());

  cyc::QueryEngine* input = qe->QueryElement("input");
  cyc::QueryEngine* output = qe->QueryElement("output");

  std::string in_commod = input->GetString("incommodity");
  std::string in_recipe = input->GetString("inrecipe");
  std::string out_commod = output->GetString("outcommodity");
  double tails_assay = output->GetDouble("tails_assay");

  double inv_size = cyc::GetOptionalQuery<double>(input,
                                                  "inventorysize",
                                                  std::numeric_limits<double>::max());

  cyc::Material::Ptr feed = cyc::Material::CreateUntracked(0,
                                                           context()->GetRecipe(in_recipe));
  double feed_assay = cyc::enrichment::UraniumAssay(feed);

  double swu_cap = cyc::GetOptionalQuery<double>(output,
                                                 "swu_capacity",
                                                 std::numeric_limits<double>::max());

  double initial_reserves = cyc::GetOptionalQuery<double>(qe,
                                                          "initial_condition/reserves_qty",
                                                          0);
  di.NewDatum("Info")
  ->AddVal("in_commod", in_commod)
  ->AddVal("in_recipe", in_recipe)
  ->AddVal("out_commod", out_commod)
  ->AddVal("tails_assay", tails_assay)
  ->AddVal("inv_size", inv_size)
  ->AddVal("feed_assay", feed_assay)
  ->AddVal("swu_cap", swu_cap)
  ->AddVal("initial_reserves", initial_reserves)
  ->AddVal("current_swu_capacity", static_cast<double>(0))
  ->Record();
}

void EnrichmentFacility::InitFrom(cyc::QueryBackend* b) {
  cyc::FacilityModel::InitFrom(b);

  cyc::QueryResult qr = b->Query("Info", NULL);

  in_commod_ = qr.GetVal<std::string>("in_commod");
  in_recipe_ = qr.GetVal<std::string>("in_recipe");
  out_commod_ = qr.GetVal<std::string>("out_commod");
  tails_assay_ = qr.GetVal<double>("tails_assay");
  feed_assay_ = qr.GetVal<double>("feed_assay");
  swu_capacity_ = qr.GetVal<double>("swu_cap");
  current_swu_capacity_ = qr.GetVal<double>("current_swu_capacity");
  initial_reserves_ = qr.GetVal<double>("initial_reserves");
  inventory_.set_capacity(qr.GetVal<double>("inv_size"));
}

void EnrichmentFacility::Snapshot(cyc::DbInit di) {
  cyc::FacilityModel::Snapshot(di);
  di.NewDatum("Info")
  ->AddVal("in_commod", in_commod_)
  ->AddVal("in_recipe", in_recipe_)
  ->AddVal("out_commod", out_commod_)
  ->AddVal("tails_assay", tails_assay_)
  ->AddVal("inv_size", inventory_.capacity())
  ->AddVal("feed_assay", feed_assay_)
  ->AddVal("swu_cap", swu_capacity_)
  ->AddVal("initial_reserves", initial_reserves_)
  ->AddVal("current_swu_capacity", current_swu_capacity_)
  ->Record();
}

void EnrichmentFacility::InitInv(cyc::Inventories& inv) {
  inventory_.PushAll(inv["inventory"]);
}

cyc::Inventories EnrichmentFacility::SnapshotInv() {
  cyc::Inventories invs;
  invs["inventory"] = inventory_.PopN(inventory_.count());
  return invs;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Model* EnrichmentFacility::Clone() {
  EnrichmentFacility* m = new EnrichmentFacility(context());
  m->InitFrom(this);
  LOG(cyc::LEV_DEBUG1, "EnrFac") << "Cloned - " << str();
  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::InitFrom(EnrichmentFacility* m) {
  FacilityModel::InitFrom(m);

  initial_reserves_ = m->initial_reserves_;
  tails_assay_ = m->tails_assay_;
  feed_assay_ = m->feed_assay_;
  in_commod_ = m->in_commod_;
  in_recipe_ = m->in_recipe_;
  out_commod_ = m->out_commod_;
  current_swu_capacity_ = m->current_swu_capacity_;
  inventory_.set_capacity(m->inventory_.capacity());
  swu_capacity_ = m->swu_capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string EnrichmentFacility::str() {
  std::stringstream ss;
  ss << cyc::FacilityModel::str()
     << " with enrichment facility parameters:"
     << " * SWU capacity: " << swu_capacity()
     << " * Tails assay: " << tails_assay()
     << " * Feed assay: " << feed_assay()
     << " * Input cyc::Commodity: " << in_commodity()
     << " * Output cyc::Commodity: " << out_commodity();
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Build(cyc::Model* parent) {
  using cyc::Material;

  FacilityModel::Build(parent);
  if (initial_reserves_ > 0) {
    inventory_.Push(
      Material::Create(
        this, initial_reserves_, context()->GetRecipe(in_recipe_)));
  }

  LOG(cyc::LEV_DEBUG2, "EnrFac") << "EnrichmentFacility "
                                 << " entering the simuluation: ";
  LOG(cyc::LEV_DEBUG2, "EnrFac") << str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Tick(int time) {
  LOG(cyc::LEV_INFO3, "EnrFac") << prototype() << " is ticking {";
  LOG(cyc::LEV_INFO3, "EnrFac") << "}";
  current_swu_capacity_ = swu_capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Tock(int time) {
  LOG(cyc::LEV_INFO3, "EnrFac") << prototype() << " is tocking {";
  LOG(cyc::LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyc::RequestPortfolio<cyc::Material>::Ptr>
EnrichmentFacility::GetMatlRequests() {
  using cyc::CapacityConstraint;
  using cyc::Material;
  using cyc::RequestPortfolio;
  using cyc::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat = Request_();
  double amt = mat->quantity();

  if (amt > cyc::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    port->AddRequest(mat, this, in_commod_);

    ports.insert(port);
  } // if amt > eps

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AcceptMatlTrades(
  const std::vector< std::pair<cyc::Trade<cyc::Material>,
  cyc::Material::Ptr> >& responses) {
  // see
  // http://stackoverflow.com/questions/5181183/boostshared-ptr-and-inheritance
  std::vector< std::pair<cyc::Trade<cyc::Material>,
      cyc::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    AddMat_(it->second);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyc::BidPortfolio<cyc::Material>::Ptr>
EnrichmentFacility::GetMatlBids(
  const cyc::CommodMap<cyc::Material>::type& commod_requests) {
  using cyc::Bid;
  using cyc::BidPortfolio;
  using cyc::CapacityConstraint;
  using cyc::Converter;
  using cyc::Material;
  using cyc::Request;

  std::set<BidPortfolio<Material>::Ptr> ports;

  if (commod_requests.count(out_commod_) > 0 && inventory_.quantity() > 0) {
    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    const std::vector<Request<Material>::Ptr>& requests = commod_requests.at(
                                                            out_commod_);

    std::vector<Request<Material>::Ptr>::const_iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      const Request<Material>::Ptr req = *it;
      if (ValidReq(req->target())) {
        Material::Ptr offer = Offer_(req->target());
        port->AddBid(req, offer, this);
      }
    }

    Converter<Material>::Ptr sc(new SWUConverter(feed_assay_, tails_assay_));
    Converter<Material>::Ptr nc(new NatUConverter(feed_assay_, tails_assay_));
    CapacityConstraint<Material> swu(swu_capacity_, sc);
    CapacityConstraint<Material> natu(inventory_.quantity(), nc);
    port->AddConstraint(swu);
    port->AddConstraint(natu);

    LOG(cyc::LEV_INFO5, "EnrFac") << prototype()
                                  << " adding a swu constraint of "
                                  << swu.capacity();
    LOG(cyc::LEV_INFO5, "EnrFac") << prototype()
                                  << " adding a natu constraint of "
                                  << natu.capacity();

    ports.insert(port);
  }
  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EnrichmentFacility::ValidReq(const cyc::Material::Ptr mat) {
  cyc::MatQuery q(mat);
  double u235 = q.atom_frac(922350000);
  double u238 = q.atom_frac(922380000);
  return (u238 > 0 && u235 / (u235 + u238) > tails_assay());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::GetMatlTrades(
  const std::vector< cyc::Trade<cyc::Material> >& trades,
  std::vector<std::pair<cyc::Trade<cyc::Material>,
  cyc::Material::Ptr> >& responses) {
  using cyc::Material;
  using cyc::Trade;

  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    Material::Ptr mat = it->bid->offer();
    double qty = it->amt;
    Material::Ptr response = Enrich_(mat, qty);
    responses.push_back(std::make_pair(*it, response));
    LOG(cyc::LEV_INFO5, "EnrFac") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << out_commod_;
  }

  if (cyc::IsNegative(current_swu_capacity_)) {
    throw cyc::ValueError(
      "EnrFac " + prototype()
      + " is being asked to provide more than its SWU capacity.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AddMat_(cyc::Material::Ptr mat) {
  if (mat->comp() != context()->GetRecipe(in_recipe_)) {
    throw cyc::ValueError(
      "EnrichmentFacility recipe and material composition not the same.");
  }

  LOG(cyc::LEV_INFO5, "EnrFac") << prototype() << " is initially holding "
                                << inventory_.quantity() << " total.";

  try {
    inventory_.Push(mat);
  } catch (cyc::Error& e) {
    e.msg(Model::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyc::LEV_INFO5, "EnrFac") << prototype() << " added " << mat->quantity()
                                << " of " << in_commod_
                                << " to its inventory, which is holding "
                                << inventory_.quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Material::Ptr EnrichmentFacility::Request_() {
  double qty = std::max(0.0, MaxInventorySize() - InventorySize());
  return cyc::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe_));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Material::Ptr EnrichmentFacility::Offer_(cyc::Material::Ptr mat) {
  cyc::MatQuery q(mat);
  cyc::CompMap comp;
  comp[922350000] = q.atom_frac(922350000);
  comp[922380000] = q.atom_frac(922380000);
  return cyc::Material::CreateUntracked(
           mat->quantity(), cyc::Composition::CreateFromAtom(comp));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyc::Material::Ptr EnrichmentFacility::Enrich_(
  cyc::Material::Ptr mat,
  double qty) {
  using cyc::Material;
  using cyc::ResCast;
  using cyc::enrichment::Assays;
  using cyc::enrichment::UraniumAssay;
  using cyc::enrichment::SwuRequired;
  using cyc::enrichment::FeedQty;
  using cyc::enrichment::TailsQty;

  // get enrichment parameters
  Assays assays(feed_assay(), UraniumAssay(mat), tails_assay());
  double swu_req = SwuRequired(qty, assays);
  double natu_req = FeedQty(qty, assays);

  // pop amount from inventory and blob it into one material
  std::vector<Material::Ptr> manifest;
  try {
    // required so popping doesn't take out too much
    if (cyc::AlmostEq(natu_req, inventory_.quantity())) {
      manifest = ResCast<Material>(inventory_.PopN(inventory_.count()));
    } else {
      manifest = ResCast<Material>(inventory_.PopQty(natu_req));
    }
  } catch (cyc::Error& e) {
    NatUConverter nc(feed_assay_, tails_assay_);
    std::stringstream ss;
    ss << " tried to remove " << natu_req
       << " from its inventory of size " << inventory_.quantity()
       << " and the conversion of the material into natu is " << nc.convert(mat);
    throw cyc::ValueError(Model::InformErrorMsg(ss.str()));
  }
  Material::Ptr r = manifest[0];
  for (int i = 1; i < manifest.size(); ++i) {
    r->Absorb(manifest[i]);
  }

  // "enrich" it, but pull out the composition and quantity we require from the
  // blob
  cyc::Composition::Ptr comp = mat->comp();
  Material::Ptr response = r->ExtractComp(qty, comp);

  current_swu_capacity_ -= swu_req;

  RecordEnrichment_(natu_req, swu_req);

  LOG(cyc::LEV_INFO5, "EnrFac") << prototype() <<
                                " has performed an enrichment: ";
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Feed Qty: "
                                << natu_req;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Feed Assay: "
                                << assays.Feed() * 100;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Product Qty: "
                                << qty;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Product Assay: "
                                << assays.Product() * 100;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Tails Qty: "
                                << TailsQty(qty, assays);
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Tails Assay: "
                                << assays.Tails() * 100;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * SWU: "
                                << swu_req;
  LOG(cyc::LEV_INFO5, "EnrFac") << "   * Current SWU capacity: "
                                << current_swu_capacity_;

  return response;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::RecordEnrichment_(double natural_u, double swu) {
  using cyc::Context;
  using cyc::Model;

  LOG(cyc::LEV_DEBUG1, "EnrFac") << prototype() << " has enriched a material:";
  LOG(cyc::LEV_DEBUG1, "EnrFac") << "  * Amount: " << natural_u;
  LOG(cyc::LEV_DEBUG1, "EnrFac") << "  *    SWU: " << swu;

  Context* ctx = Model::context();
  ctx->NewDatum("Enrichments")
  ->AddVal("ID", id())
  ->AddVal("Time", ctx->time())
  ->AddVal("Natural_Uranium", natural_u)
  ->AddVal("SWU", swu)
  ->Record();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyc::Model* ConstructEnrichmentFacility(cyc::Context* ctx) {
  return new EnrichmentFacility(ctx);
}

} // namespace cycamore


