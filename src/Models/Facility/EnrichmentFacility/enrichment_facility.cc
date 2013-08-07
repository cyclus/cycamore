// enrichment_facility.cc
// Implements the EnrichmentFacility class
#include "enrichment_facility.h"

#include "MarketModel.h"
#include "QueryEngine.h"
#include "Logger.h"
#include "error.h"
#include "CycLimits.h"
#include "GenericResource.h"
#include "Material.h"
#include "Timer.h"
#include "EventManager.h"

#include <sstream>
#include <limits>
#include <cmath>

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using boost::lexical_cast;

int EnrichmentFacility::entry_ = 0;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::EnrichmentFacility() :
  commodity_price_(0),
  tails_assay_(0),
  feed_assay_(0),
  in_commodity_(""),
  in_recipe_(""),
  out_commodity_("")
{}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::~EnrichmentFacility() 
{}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::initModuleMembers(cyclus::QueryEngine* qe) 
{
  string data;

  cyclus::QueryEngine* input = qe->queryElement("input");
  set_in_commodity(input->getElementContent("incommodity"));
  set_in_recipe(input->getElementContent("inrecipe"));

  try 
    {
      data = input->getElementContent("inventorysize"); 
      setMaxInventorySize(lexical_cast<double>(data));
    }
  catch (cyclus::Error e) 
    {
      setMaxInventorySize(numeric_limits<double>::max());
    }

  cyclus::QueryEngine* output = qe->queryElement("output");
  set_out_commodity(output->getElementContent("outcommodity"));

  data = output->getElementContent("tails_assay");
  set_tails_assay(lexical_cast<double>(data));

  cyclus::mat_rsrc_ptr feed = cyclus::mat_rsrc_ptr(new cyclus::Material(cyclus::RecipeLibrary::Recipe(in_recipe())));
  set_feed_assay(cyclus::enrichment::uranium_assay(feed));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::str() 
{
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
void EnrichmentFacility::cloneModuleMembersFrom(cyclus::FacilityModel* sourceModel) 
{
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
  set_tails_assay(source->tails_assay());
  set_feed_assay(source->feed_assay());
  set_in_commodity(source->in_commodity());
  set_in_recipe(source->in_recipe());
  set_out_commodity(source->out_commodity());
  setMaxInventorySize(source->maxInventorySize());
  set_commodity_price(source->commodity_price());

  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "Cloned - " << str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::addResource(cyclus::Transaction trans, std::vector<cyclus::rsrc_ptr> manifest) 
{
  LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " adding material qty: " << manifest.at(0)->quantity();
  inventory_.pushAll(cyclus::MatBuff::toMat(manifest));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::vector<cyclus::rsrc_ptr> EnrichmentFacility::removeResource(cyclus::Transaction order) 
{
  cyclus::rsrc_ptr prsrc = order.resource();
  if (!cyclus::Material::isMaterial(prsrc)) 
    throw cyclus::CastError("Can't remove a resource as a non-material");
  
  cyclus::mat_rsrc_ptr rsrc = dynamic_pointer_cast<cyclus::Material>(prsrc);

  cyclus::enrichment::Assays assays = getAssays(rsrc);
  double product_qty = cyclus::enrichment::uranium_qty(rsrc);
  double swu = swu_required(product_qty,assays);
  double natural_u = feed_qty(product_qty,assays);
  inventory_.popQty(natural_u);

  
  LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " has performed an enrichment: ";
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Qty: " << feed_qty(product_qty,assays); 
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Feed Assay: " << assays.feed() * 100; 
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Qty: " << product_qty;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Product Assay: " << assays.product() * 100;
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Qty: " << tails_qty(product_qty,assays); 
  LOG(cyclus::LEV_INFO5, "EnrFac") << "   * Tails Assay: " << assays.tails() * 100; 
  recordEnrichment(natural_u,swu);

  vector<cyclus::rsrc_ptr> ret;
  ret.push_back(order.resource());
  return ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::receiveMessage(cyclus::msg_ptr msg)
{
  // is this a message from on high? 
  if(msg->trans().supplier() == this)
    {
      // file the order
      orders_.push_back(msg);
      LOG(cyclus::LEV_INFO5, "EnrFac") << name() << " just received an order for: ";
      msg->trans().resource()->print();
    } 
  else 
    {
      throw cyclus::Error("EnrFacility is not the supplier of this msg.");
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTick(int time)
{
  LOG(cyclus::LEV_INFO3, "EnrFac") << facName() << " is ticking {";

  makeRequest();
  makeOffer();

  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTock(int time)
{
  LOG(cyclus::LEV_INFO3, "EnrFac") << facName() << " is tocking {";

  processOutgoingMaterial();

  LOG(cyclus::LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::makeRequest()
{
  double amt = inventory_.space();
  double min_amt = 0;
  string commodity = in_commodity();

  if (amt > cyclus::eps())
    {
      LOG(cyclus::LEV_INFO4, "EnrFac") << " requests "<< amt << " kg of " << commodity << ".";

      cyclus::MarketModel* market = cyclus::MarketModel::marketForCommod(commodity);
      Communicator* recipient = dynamic_cast<Communicator*>(market);

      // create a material resource
      // @MJGFlag note that this doesn't matter in the current state
      cyclus::mat_rsrc_ptr request_res = 
        cyclus::mat_rsrc_ptr(new cyclus::Material(cyclus::RecipeLibrary::Recipe(in_recipe_)));
      request_res->setQuantity(amt);
      
      // build the transaction and message
      cyclus::Transaction trans(this, cyclus::REQUEST);
      trans.setCommod(commodity);
      trans.setMinFrac(min_amt/amt);
      trans.setPrice(commodity_price_);
      trans.setResource(request_res);

      cyclus::msg_ptr request(new cyclus::Message(this, recipient, trans)); 
      request->sendOn();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::Transaction EnrichmentFacility::buildTransaction() 
{
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  cyclus::mat_rsrc_ptr offer_res = cyclus::mat_rsrc_ptr(new cyclus::Material());
  offer_res->setQuantity(offer_amt);
  cyclus::Transaction trans(this, cyclus::OFFER);

  trans.setCommod(out_commodity());
  trans.setMinFrac(min_amt/offer_amt);
  trans.setPrice(commodity_price());
  trans.setResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::makeOffer() 
{
  cyclus::MarketModel* market = cyclus::MarketModel::marketForCommod(out_commodity());

  Communicator* recipient = dynamic_cast<Communicator*>(market);


  // note that this is a hack. the amount of the resource being offered
  // is greater than the possible amount that can be serviced
  cyclus::Transaction trans = buildTransaction();

  cyclus::msg_ptr msg(new cyclus::Message(this, recipient, trans)); 

  if (trans.resource()->quantity() > 0) 
    {
      LOG(cyclus::LEV_INFO4, "EnrFac") << "offers "<< trans.resource()->quantity() << " kg of "
                               << out_commodity_ << ".";

      msg->sendOn();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::processOutgoingMaterial()
{
  double remove_total = 0;
  while (!orders_.empty())
    {
      cyclus::Transaction trans = orders_.front()->trans();

      cyclus::rsrc_ptr prsrc = trans.resource();
      if (!cyclus::Material::isMaterial(prsrc)) 
       throw cyclus::CastError("Can't process a resource as a non-material");

      cyclus::mat_rsrc_ptr rsrc = dynamic_pointer_cast<cyclus::Material>(prsrc);

      LOG(cyclus::LEV_DEBUG1, "EnrFac") << "Processing material: ";
      rsrc->print();

      cyclus::enrichment::Assays assays = getAssays(rsrc);
      double product_qty = cyclus::enrichment::uranium_qty(rsrc);
      remove_total += feed_qty(product_qty,assays);

      if (remove_total < inventory_.quantity())
        {
          trans.approveTransfer();
        }
      // else
      //   {
      //     throw cyclus::CycOverrideException("Can't process more than an EnrFac's inventory size");
      //   }

      orders_.pop_front();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
cyclus::enrichment::Assays EnrichmentFacility::getAssays(cyclus::mat_rsrc_ptr rsrc)
{
  double product_assay = cyclus::enrichment::uranium_assay(rsrc);
  return cyclus::enrichment::Assays(feed_assay(),product_assay,tails_assay());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::recordEnrichment(double natural_u, double swu)
{
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << name() << " has enriched a material:";
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  * Amount: " << natural_u;
  LOG(cyclus::LEV_DEBUG1, "EnrFac") << "  *    SWU: " << swu;

  cyclus::EM->newEvent("Enrichments")
    ->addVal("ENTRY", ++entry_)
    ->addVal("ID", ID())
    ->addVal("Time", cyclus::TI->time())
    ->addVal("Natural_Uranium", natural_u)
    ->addVal("SWU", swu)
    ->record();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" cyclus::Model* constructEnrichmentFacility() 
{
  return new EnrichmentFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructEnrichmentFacility(cyclus::Model* model) 
{
      delete model;
}
