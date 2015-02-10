// Implements the EnrichmentFacility class
#include "enrichment_facility.h"
#include "behavior_functions.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include <boost/lexical_cast.hpp>

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::EnrichmentFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      tails_assay(0),
      feed_assay(0),
      swu_capacity(0),
      social_behav(0), //***
      initial_reserves(0),
      in_commod(""),
      in_recipe(""),
      out_commods() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::~EnrichmentFacility() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string EnrichmentFacility::str() {
  std::stringstream ss;

  std::string out_commod_msg = "";
  out_commod_msg += " * Output cyclus::Commodities: " ;
  for (std::vector<std::string>::iterator commod = out_commods.begin();
       commod != out_commods.end();
       commod++) {
    out_commod_msg += (commod == out_commods.begin() ? "{" : ", ");
    out_commod_msg += (*commod);
  }

  ss << cyclus::Facility::str()
     << " with enrichment facility parameters:"
     << " * SWU capacity: " << SwuCapacity()
     << " * Tails assay: " << TailsAssay()
     << " * Feed assay: " << FeedAssay()
     << " * Input cyclus::Commodity: " << in_commodity()
     << out_commod_msg ;
  return ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Build(cyclus::Agent* parent) {
  using cyclus::Material;

  Facility::Build(parent);
  if (initial_reserves > 0) {
    inventory.Push(
      Material::Create(
        this, initial_reserves, context()->GetRecipe(in_recipe)));
  }

  LOG(cyclus::LEV_DEBUG2, "EnrFac") << "EnrichmentFacility "
                                 << " entering the simuluation: ";
  LOG(cyclus::LEV_DEBUG2, "EnrFac") << str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Tick() {
  LOG(cyclus::LEV_INFO3, "EnrFac") << prototype() << " is ticking {";
  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
  current_swu_capacity = SwuCapacity();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::Tock() {
  LOG(cyclus::LEV_INFO3, "EnrFac") << prototype() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
EnrichmentFacility::GetMatlRequests() {
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat = Request_();
  double amt = mat->quantity();

  if (amt > cyclus::eps()) {
    port->AddRequest(mat, this, in_commod);
    ports.insert(port);
  }

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AcceptMatlTrades(
  const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  // see
  // http://stackoverflow.com/questions/5181183/boostshared-ptr-and-inheritance
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    AddMat_(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
EnrichmentFacility::GetMatlBids(
  cyclus::CommodMap<cyclus::Material>::type& commod_requests) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;

  std::set<BidPortfolio<Material>::Ptr> ports;
  // *** Add to modify preferences for specific timesteps ***//
  if (social_behav) {
    int cur_time = context()->time();
    //  only trade on every 5th timestep
    int interval = 5 ;
    if (EveryXTimestep(cur_time, interval)) {
      return ports;
    //    if (cur_time % 5 != 0) {
    //      return ports; 
    }
  }    
  if (inventory.quantity() <= 0) {
    return ports;
  }
  
  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());
  
  for (std::vector<std::string>::iterator commod = out_commods.begin();
       commod != out_commods.end();
       ++commod) {
    if (commod_requests.count(*commod) == 0) {
      continue;
    }
  
    std::vector<Request<Material>*>& requests =
      commod_requests[*commod];
    
    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      if (ValidReq(req->target())) {
	Material::Ptr offer = Offer_(req->target());
	port->AddBid(req, offer, this);
      }
    }
  } //for each out commod
  
  Converter<Material>::Ptr sc(new SWUConverter(feed_assay, tails_assay));
  Converter<Material>::Ptr nc(new NatUConverter(feed_assay, tails_assay));
  CapacityConstraint<Material> swu(swu_capacity, sc);
  CapacityConstraint<Material> natu(inventory.quantity(), nc);
  port->AddConstraint(swu);
  port->AddConstraint(natu);
  
  LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				   << " adding a swu constraint of "
				   << swu.capacity();
  LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				   << " adding a natu constraint of "
				   << natu.capacity(); 
  ports.insert(port);
  
  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool EnrichmentFacility::ValidReq(const cyclus::Material::Ptr mat) {
  cyclus::toolkit::MatQuery q(mat);
  double u235 = q.atom_frac(922350000);
  double u238 = q.atom_frac(922380000);
  return (u238 > 0 && u235 / (u235 + u238) > TailsAssay());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    Material::Ptr mat = it->bid->offer();
    double qty = it->amt;
    Material::Ptr response = Enrich_(mat, qty);
    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				     << " just received an order"
				     << " for " << it->amt
				     << " of " << it->bid->request()->commodity() ;
  }

  if (cyclus::IsNegative(current_swu_capacity)) {
    throw cyclus::ValueError(
      "EnrFac " + prototype()
      + " is being asked to provide more than its SWU capacity.");
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AddMat_(cyclus::Material::Ptr mat) {
  if (mat->comp() != context()->GetRecipe(in_recipe)) {
    throw cyclus::ValueError(
      "EnrichmentFacility recipe and material composition not the same.");
  }

  LOG(cyclus::LEV_INFO5, "EnrFac") << prototype() << " is initially holding "
                                << inventory.quantity() << " total.";

  try {
    inventory.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "EnrFac") << prototype() << " added "
                                   << mat->quantity() << " of " << in_commod
                                   << " to its inventory, which is holding "
                                   << inventory.quantity() << " total.";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacility::Request_() {
  double qty = std::max(0.0, MaxInventorySize() - InventorySize());
  return cyclus::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacility::Offer_(cyclus::Material::Ptr mat) {
  cyclus::toolkit::MatQuery q(mat);
  cyclus::CompMap comp;
  comp[922350000] = q.atom_frac(922350000);
  comp[922380000] = q.atom_frac(922380000);
  return cyclus::Material::CreateUntracked(
           mat->quantity(), cyclus::Composition::CreateFromAtom(comp));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr EnrichmentFacility::Enrich_(
  cyclus::Material::Ptr mat,
  double qty) {
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::Assays;
  using cyclus::toolkit::UraniumAssay;
  using cyclus::toolkit::SwuRequired;
  using cyclus::toolkit::FeedQty;
  using cyclus::toolkit::TailsQty;

  // get enrichment parameters
  Assays assays(FeedAssay(), UraniumAssay(mat), TailsAssay());
  double swu_req = SwuRequired(qty, assays);
  double natu_req = FeedQty(qty, assays);

  // pop amount from inventory and blob it into one material
  std::vector<Material::Ptr> manifest;
  try {
    // required so popping doesn't take out too much
    if (cyclus::AlmostEq(natu_req, inventory.quantity())) {
      manifest = ResCast<Material>(inventory.PopN(inventory.count()));
    } else {
      manifest = ResCast<Material>(inventory.PopQty(natu_req));
    }
  } catch (cyclus::Error& e) {
    NatUConverter nc(feed_assay, tails_assay);
    std::stringstream ss;
    ss << " tried to remove " << natu_req
       << " from its inventory of size " << inventory.quantity()
       << " and the conversion of the material into natu is "
       << nc.convert(mat);
    throw cyclus::ValueError(Agent::InformErrorMsg(ss.str()));
  }
  Material::Ptr r = manifest[0];
  for (int i = 1; i < manifest.size(); ++i) {
    r->Absorb(manifest[i]);
  }

  // "enrich" it, but pull out the composition and quantity we require from the
  // blob
  cyclus::Composition::Ptr comp = mat->comp();
  Material::Ptr response = r->ExtractComp(qty, comp);
  tails.Push(r); // add remainder to tails buffer
  
  current_swu_capacity -= swu_req;

  RecordEnrichment_(natu_req, swu_req);

  LOG(cyclus::LEV_INFO5, "EnrFac") << prototype() <<
                                " has performed an enrichment: ";
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Qty: "
                                << natu_req;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Assay: "
                                << assays.Feed() * 100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Qty: "
                                << qty;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Assay: "
                                << assays.Product() * 100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Qty: "
                                << TailsQty(qty, assays);
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Assay: "
                                << assays.Tails() * 100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * SWU: "
                                << swu_req;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Current SWU capacity: "
                                << CurrentSwuCapacity();

  return response;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::RecordEnrichment_(double natural_u, double swu) {
  using cyclus::Context;
  using cyclus::Agent;

  LOG(cyclus::LEV_DEBUG1, "EnrFac") << prototype()
                                    << " has enriched a material:";
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  * Amount: " << natural_u;
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  *    SWU: " << swu;

  Context* ctx = Agent::context();
  ctx->NewDatum("Enrichments")
      ->AddVal("ID", id())
      ->AddVal("Time", ctx->time())
      ->AddVal("Natural_Uranium", natural_u)
      ->AddVal("SWU", swu)
      ->Record();
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
  bool EnrichmentFacility::EveryXTimestep(int curr_time, int interval) {
  return curr_time % interval != 0;
}
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructEnrichmentFacility(cyclus::Context* ctx) {
  return new EnrichmentFacility(ctx);
}
  
}  // namespace cycamore
