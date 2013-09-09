// enrichment_facility.cc
// Implements the EnrichmentFacility class
#include "enrichment_facility.h"

#include "market_model.h"
#include "query_engine.h"
#include "logger.h"
#include "error.h"
#include "context.h"
#include "cyc_limits.h"
#include "generic_resource.h"
#include "material.h"
#include "timer.h"

#include <sstream>
#include <limits>
#include <cmath>

#include <boost/lexical_cast.hpp>

namespace cycamore {

int EnrichmentFacility::entry_ = 0;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::EnrichmentFacility(cyclus::Context* ctx)
    : cyclus::FacilityModel(ctx),
      commodity_price_(0),
      tails_assay_(0),
      feed_assay_(0),
      in_commodity_(""),
      in_recipe_(""),
      out_commodity_("") { }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
EnrichmentFacility::~EnrichmentFacility() { }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  using std::numeric_limits;
  using boost::lexical_cast;
  using cyclus::Model;
  using cyclus::Material;
  
  string data;

  cyclus::QueryEngine* input = qe->QueryElement("input");
  set_in_commodity(input->GetElementContent("incommodity"));
  set_in_recipe(input->GetElementContent("inrecipe"));

  double limit = 
      cyclus::GetOptionalQuery<double>(input, "inventorysize", numeric_limits<double>::max());
  SetMaxInventorySize(limit);

  cyclus::QueryEngine* output = qe->QueryElement("output");
  set_out_commodity(output->GetElementContent("outcommodity"));

  data = output->GetElementContent("tails_assay");
  set_tails_assay(lexical_cast<double>(data));

  cyclus::Context* ctx = Model::context();
  Material::Ptr feed = Material::Create(ctx, 0, ctx->GetRecipe(in_recipe_));
  set_feed_assay(cyclus::enrichment::UraniumAssay(feed));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string EnrichmentFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str()
     << " with enrichment facility parameters:"
     << " * Tails assay: " << tails_assay()
     << " * Feed assay: " << feed_assay()
     << " * Input cyclus::Commodity: " << in_commodity()
     << " * Output cyclus::Commodity: " << out_commodity();
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::CloneModuleMembersFrom(cyclus::FacilityModel*
                                                sourceModel) {
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
  set_tails_assay(source->tails_assay());
  set_feed_assay(source->feed_assay());
  set_in_commodity(source->in_commodity());
  set_in_recipe(source->in_recipe());
  set_out_commodity(source->out_commodity());
  SetMaxInventorySize(source->MaxInventorySize());
  set_commodity_price(source->commodity_price());

  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "Cloned - " << str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::AddResource(cyclus::Transaction trans,
                                     std::vector<cyclus::Resource::Ptr> manifest) {
  LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " adding material qty: " <<
                                   manifest.at(0)->quantity();
  inventory_.PushAll(cyclus::MatBuff::ToMat(manifest));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::vector<cyclus::Resource::Ptr> EnrichmentFacility::RemoveResource(
  cyclus::Transaction order) {
  using std::vector;
  using boost::dynamic_pointer_cast;
  cyclus::Resource::Ptr prsrc = order.resource();

  cyclus::Material::Ptr rsrc = dynamic_pointer_cast<cyclus::Material>(prsrc);
  if (rsrc == 0) {
    throw cyclus::CastError("Can't remove a resource as a non-material");
  }

  cyclus::enrichment::Assays assays = GetAssays(rsrc);
  double product_qty = cyclus::enrichment::UraniumQty(rsrc);
  double swu = cyclus::enrichment::SwuRequired(product_qty, assays);
  double natural_u = cyclus::enrichment::FeedQty(product_qty, assays);
  inventory_.PopQty(natural_u);


  LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " has performed an enrichment: ";
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Qty: " <<
                                   cyclus::enrichment::FeedQty(product_qty, assays);
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Assay: " << assays.Feed() * 100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Qty: " << product_qty;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Assay: " << assays.Product() *
                                   100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Qty: " <<
                                   cyclus::enrichment::TailsQty(product_qty, assays);
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Assay: " << assays.Tails() *
                                   100;
  RecordEnrichment(natural_u, swu);

  vector<cyclus::Resource::Ptr> ret;
  ret.push_back(order.resource());
  return ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::ReceiveMessage(cyclus::Message::Ptr msg) {
  // is this a message from on high?
  if (msg->trans().supplier() == this) {
    // file the order
    orders_.push_back(msg);
    LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " just received an order for: ";
  } else {
    throw cyclus::Error("EnrFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "EnrFac") << FacName() << " is ticking {";

  MakeRequest();
  MakeOffer();

  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "EnrFac") << FacName() << " is tocking {";

  ProcessOutgoingMaterial();

  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::MakeRequest() {
  using std::string;
  using cyclus::Model;
  using cyclus::Material;

  double amt = inventory_.space();
  double min_amt = 0;
  string commodity = in_commodity();

  if (amt > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "EnrFac") << " requests " << amt << " kg of " <<
                                     commodity << ".";

    cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(commodity);
    Communicator* recipient = dynamic_cast<Communicator*>(market);

    // create a material resource
    // @MJGFlag note that this doesn't matter in the current state
    cyclus::Context* ctx = Model::context();
    Material::Ptr request_res =
        Material::Create(ctx, amt, ctx->GetRecipe(in_recipe_));

    // build the transaction and message
    cyclus::Transaction trans(this, cyclus::REQUEST);
    trans.SetCommod(commodity);
    trans.SetMinFrac(min_amt / amt);
    trans.SetPrice(commodity_price_);
    trans.SetResource(request_res);

    cyclus::Message::Ptr request(new cyclus::Message(this, recipient, trans));
    request->SendOn();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Transaction EnrichmentFacility::BuildTransaction() {
  using cyclus::Model;
  using cyclus::Material;

  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  cyclus::Context* ctx = Model::context();
  cyclus::Material::Ptr offer_res =
      cyclus::Material::Create(ctx, offer_amt, ctx->GetRecipe(in_commodity_));
  cyclus::Transaction trans(this, cyclus::OFFER);

  trans.SetCommod(out_commodity());
  trans.SetMinFrac(min_amt / offer_amt);
  trans.SetPrice(commodity_price());
  trans.SetResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::MakeOffer() {
  cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(
                                  out_commodity());

  Communicator* recipient = dynamic_cast<Communicator*>(market);


  // note that this is a hack. the amount of the resource being offered
  // is greater than the possible amount that can be serviced
  cyclus::Transaction trans = BuildTransaction();

  cyclus::Message::Ptr msg(new cyclus::Message(this, recipient, trans));

  if (trans.resource()->quantity() > 0) {
    LOG(cyclus::LEV_INFO4, "EnrFac") << "offers " << trans.resource()->quantity() <<
                                     " kg of "
                                     << out_commodity_ << ".";

    msg->SendOn();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::ProcessOutgoingMaterial() {
  using boost::dynamic_pointer_cast;
  double remove_total = 0;
  while (!orders_.empty()) {
    cyclus::Transaction trans = orders_.front()->trans();

    cyclus::Resource::Ptr prsrc = trans.resource();
    cyclus::Material::Ptr rsrc = dynamic_pointer_cast<cyclus::Material>(prsrc);
    if (rsrc == 0) {
      throw cyclus::CastError("Can't remove a resource as a non-material");
    }

    LOG(cyclus::LEV_DEBUG1, "EnrFac") << "Processing material: ";

    cyclus::enrichment::Assays assays = GetAssays(rsrc);
    double product_qty = cyclus::enrichment::UraniumQty(rsrc);
    remove_total += cyclus::enrichment::FeedQty(product_qty, assays);

    if (remove_total < inventory_.quantity()) {
      trans.ApproveTransfer();
    }
    // else
    //   {
    //     throw cyclus::CycOverrideException("Can't process more than an EnrFac's inventory size");
    //   }

    orders_.pop_front();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::enrichment::Assays EnrichmentFacility::GetAssays(
  cyclus::Material::Ptr rsrc) {
  double product_assay = cyclus::enrichment::UraniumAssay(rsrc);
  return cyclus::enrichment::Assays(feed_assay(), product_assay, tails_assay());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void EnrichmentFacility::RecordEnrichment(double natural_u, double swu) {
  using cyclus::Context;
  using cyclus::Model;
  
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << name() << " has enriched a material:";
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  * Amount: " << natural_u;
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  *    SWU: " << swu;

  Context* ctx = Model::context();
  ctx->NewEvent("Enrichments")
      ->AddVal("ENTRY", ++entry_)
      ->AddVal("ID", ID())
      ->AddVal("Time", ctx->time())
      ->AddVal("Natural_Uranium", natural_u)
      ->AddVal("SWU", swu)
      ->Record();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructEnrichmentFacility(cyclus::Context* ctx) {
  return new EnrichmentFacility(ctx);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void DestructEnrichmentFacility(cyclus::Model* model) {
  delete model;
}
} // namespace cycamore
