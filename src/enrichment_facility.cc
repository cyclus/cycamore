/*
How/Where do I check that enrichment limit is a fraction of 1?
Where is the default max enrich set?
Use MaxEnrich or max_enrich?
*/

// Implements the EnrichmentFacility class
#include "enrichment_facility.h"

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
      swu_capacity(0),
      max_enrich(0),  ///QQ is this defaulting to zero? Where to set default?
      initial_reserves(0),
      in_commod(""),
      in_recipe(""),
      out_commod(""),
      tails_commod(""){}   ///QQ

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::~EnrichmentFacility() {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string EnrichmentFacility::str() {
  std::stringstream ss;
  ss << cyclus::Facility::str()
     << " with enrichment facility parameters:"
     << " * SWU capacity: " << SwuCapacity()
     << " * Tails assay: " << TailsAssay()
     << " * Feed assay: " << FeedAssay()   //QQ Remove??
     << " * Input cyclus::Commodity: " << in_commodity()
     << " * Output cyclus::Commodity: " << out_commodity()
     << " * Tails cyclus::Commodity: " << tails_commodity(); ///QQ
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

  ///QQ Here is where we check material composition 
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
EnrichmentFacility::GetMatlBids(
    cyclus::CommodMap<cyclus::Material>::type& out_requests){
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;

  std::set<BidPortfolio<Material>::Ptr> ports;
  //QQ 
  if (out_requests.count(tails_commod) > 0 && tails.quantity() > 0) {
    BidPortfolio<Material>::Ptr tails_port(new BidPortfolio<Material>()); //QQ
    
    std::vector<Request<Material>*>& tails_requests =
      out_requests[tails_commod];
    
    std::vector<Request<Material>*>::iterator it;
    for (it = tails_requests.begin(); it != tails_requests.end(); ++it) {
      Request<Material>* req = *it;
      tails_port->AddBid(req, tails.Peek(), this);
    }
    //QQ
    // overbidding (bidding on every offer)
    // add an overall capacity constraint 
    CapacityConstraint<Material> tc(tails.quantity());
    tails_port->AddConstraint(tc);
    LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				     << " adding a tails capacity constraint of "
				     << tails.capacity();
    //QQ
    ports.insert(tails_port);
  }
  if (out_requests.count(out_commod) > 0 && inventory.quantity() > 0) {
    BidPortfolio<Material>::Ptr commod_port(new BidPortfolio<Material>()); 
    
    std::vector<Request<Material>*>& commod_requests =
      out_requests[out_commod];
    
    std::vector<Request<Material>*>::iterator it;
    for (it = commod_requests.begin(); it != commod_requests.end(); ++it) {
      Request<Material>* req = *it;
      // Do not offer a bid if the enrichment exceed max.  QQ
      Material::Ptr mat = Request_();
      double request_enrich = cyclus::toolkit::UraniumAssay(mat) ;
      if (ValidReq(req->target()) && (request_enrich <= max_enrich)) {
        Material::Ptr offer = Offer_(req->target());
        commod_port->AddBid(req, offer, this);
      }
    }
    Converter<Material>::Ptr sc(new SWUConverter(FeedAssay(), tails_assay));
    Converter<Material>::Ptr nc(new NatUConverter(FeedAssay(), tails_assay));
    CapacityConstraint<Material> swu(swu_capacity, sc);
    CapacityConstraint<Material> natu(inventory.quantity(), nc);
    commod_port->AddConstraint(swu);
    commod_port->AddConstraint(natu);
    
    LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
                                  << " adding a swu constraint of "
                                  << swu.capacity();
    LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
                                  << " adding a natu constraint of "
                                  << natu.capacity();

    ports.insert(commod_port);
  }
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
    std::string commod_type = it->bid->request()->commodity() ;
    Material::Ptr response ;
    //QQ Figure out whether material is tails or enriched,
    // if tails then make transfer of material
    
    //QQ Need to figure out if the response material is tails or product  
    if (commod_type == tails_commod){
      LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				       << " just received an order"
				       << " for " << it->amt
				       << " of " << tails_commod;
      // Do the material moving
      tails.Pop(qty);     // remove the qty from the Tails buffer
      response = mat;  //QQ Correct?
    } else {
      LOG(cyclus::LEV_INFO5, "EnrFac") << prototype()
				       << " just received an order"
				       << " for " << it->amt
				       << " of " << out_commod;
      
      response = Enrich_(mat, qty);
    }
    responses.push_back(std::make_pair(*it, response));	
  }
  if (cyclus::IsNegative(tails.quantity())) {
    std::stringstream ss;
    ss << "is being asked to provide more than its current inventory." ;
    throw cyclus::ValueError(Agent::InformErrorMsg(ss.str()));
  }
  if (cyclus::IsNegative(current_swu_capacity)) {
    throw cyclus::ValueError(
			     "EnrFac " + prototype()
			     + " is being asked to provide more than" +
			     " its SWU capacity.");
  }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AddMat_(cyclus::Material::Ptr mat) {
  // Elements and isotopes other than U-235, U-238 are sent directly to tails
  cyclus::CompMap cm = mat->comp()->atom();
  bool extra_u = false;
  bool other_elem = false;
  for (cyclus::CompMap::const_iterator it=cm.begin(); it !=cm.end(); ++it) {
    if (pyne::nucname::znum(it->first) == 92){
      if (pyne::nucname::anum(it->first) != 235 &&
          pyne::nucname::anum(it->first) != 238 && it->second > 0){
	extra_u = true;
      }
    }
    else if (it->second > 0) {
      other_elem = true ;
    }
  }
  if (extra_u){
    cyclus::Warn<cyclus::VALUE_WARNING>("More than 2 isotopes of U."  \
      "Istopes other than U-235, U-238 are sent directly to tails.");
  }
  if (other_elem){
    cyclus::Warn<cyclus::VALUE_WARNING>("Non-uranium elements are "   \
      "sent directly to tails.");
  }
  /// TODO: Add FAIL if non-235/238 quantities are too large
 

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
  Material::Ptr r;
  try {
    // required so popping doesn't take out too much
    if (cyclus::AlmostEq(natu_req, inventory.quantity())) {
      r = cyclus::toolkit::Squash(inventory.PopN(inventory.count()));
    } else {
      r = inventory.Pop(natu_req);
    }
  } catch (cyclus::Error& e) {
    NatUConverter nc(FeedAssay(), tails_assay);
    std::stringstream ss;
    ss << " tried to remove " << natu_req
       << " from its inventory of size " << inventory.quantity()
       << " and the conversion of the material into natu is "
       << nc.convert(mat);
    throw cyclus::ValueError(Agent::InformErrorMsg(ss.str()));
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
double EnrichmentFacility::FeedAssay() {
  using cyclus::Material;
  
  cyclus::Material::Ptr fission_matl=inventory.Pop(inventory.quantity());
  inventory.Push(fission_matl);
  return cyclus::toolkit::UraniumAssay(fission_matl); 
}
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructEnrichmentFacility(cyclus::Context* ctx) {
  return new EnrichmentFacility(ctx);
}

}  // namespace cycamore
