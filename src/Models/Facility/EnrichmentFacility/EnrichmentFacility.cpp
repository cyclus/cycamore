// EnrichmentFacility.cpp
// Implements the EnrichmentFacility class
#include "EnrichmentFacility.h"

#include "MarketModel.h"
#include "QueryEngine.h"
#include "Logger.h"
#include "CycException.h"
#include "GenericResource.h"
#include "Material.h"
#include "Timer.h"

#include <sstream>
#include <limits>
#include <cmath>

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using boost::lexical_cast;
using namespace enrichment;

// initialize table member
table_ptr EnrichmentFacility::table_ = table_ptr(new Table("Enrichment")); 

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
void EnrichmentFacility::initModuleMembers(QueryEngine* qe) 
{
  string data;

  QueryEngine* input = qe->queryElement("input");
  set_in_commodity(input->getElementContent("incommodity"));
  set_in_recipe(input->getElementContent("inrecipe"));

  try 
    {
      data = input->getElementContent("inventorysize"); 
      setMaxInventorySize(lexical_cast<double>(data));
    }
  catch (CycNullQueryException e) 
    {
      setMaxInventorySize(numeric_limits<double>::max());
    }

  QueryEngine* output = qe->queryElement("output");
  set_out_commodity(output->getElementContent("outcommodity"));

  data = output->getElementContent("tails_assay");
  set_tails_assay(lexical_cast<double>(data));

  mat_rsrc_ptr feed = mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(in_recipe())));
  set_feed_assay(uranium_assay(feed));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::str() 
{
  std::stringstream ss;
  ss << FacilityModel::str()
     << " with enrichment facility parameters:"
     << " * Tails assay: " << tails_assay()
     << " * Feed assay: " << feed_assay()
     << " * Input Commodity: " << in_commodity()
     << " * Output Commodity: " << out_commodity();
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::cloneModuleMembersFrom(FacilityModel* sourceModel) 
{
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
  set_tails_assay(source->tails_assay());
  set_feed_assay(source->feed_assay());
  set_in_commodity(source->in_commodity());
  set_in_recipe(source->in_recipe());
  set_out_commodity(source->out_commodity());
  setMaxInventorySize(source->maxInventorySize());
  set_commodity_price(source->commodity_price());

  LOG(LEV_DEBUG1, "EnrFac") << "Cloned - " << str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::addResource(Transaction trans, std::vector<rsrc_ptr> manifest) 
{
  LOG(LEV_INFO5, "EnrFac") << name() << " adding material qty: " << manifest.at(0)->quantity();
  inventory_.pushAll(MatBuff::toMat(manifest));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::vector<rsrc_ptr> EnrichmentFacility::removeResource(Transaction order) 
{
  rsrc_ptr prsrc = order.resource();
  if (!Material::isMaterial(prsrc)) 
    throw CycOverrideException("Can't remove a resource as a non-material");
  
  mat_rsrc_ptr rsrc = dynamic_pointer_cast<Material>(prsrc);

  Assays assays = getAssays(rsrc);
  double product_qty = uranium_qty(rsrc);
  double swu = swu_required(product_qty,assays);
  double natural_u = feed_qty(product_qty,assays);
  inventory_.popQty(natural_u);

  
  LOG(LEV_INFO5, "EnrFac") << name() << " has performed an enrichment: ";
  LOG(LEV_INFO5, "EnrFac") << "   * Feed Qty: " << feed_qty(product_qty,assays); 
  LOG(LEV_INFO5, "EnrFac") << "   * Feed Assay: " << assays.feed() * 100; 
  LOG(LEV_INFO5, "EnrFac") << "   * Product Qty: " << product_qty;
  LOG(LEV_INFO5, "EnrFac") << "   * Product Assay: " << assays.product() * 100;
  LOG(LEV_INFO5, "EnrFac") << "   * Tails Qty: " << tails_qty(product_qty,assays); 
  LOG(LEV_INFO5, "EnrFac") << "   * Tails Assay: " << assays.tails() * 100; 
  recordEnrichment(natural_u,swu);

  vector<rsrc_ptr> ret;
  ret.push_back(order.resource());
  return ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::receiveMessage(msg_ptr msg)
{
  // is this a message from on high? 
  if(msg->trans().supplier() == this)
    {
      // file the order
      orders_.push_back(msg);
      LOG(LEV_INFO5, "EnrFac") << name() << " just received an order for: ";
      msg->trans().resource()->print();
    } 
  else 
    {
      throw CycException("EnrFacility is not the supplier of this msg.");
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTick(int time)
{
  LOG(LEV_INFO3, "EnrFac") << facName() << " is ticking {";

  makeRequest();
  makeOffer();

  LOG(LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTock(int time)
{
  LOG(LEV_INFO3, "EnrFac") << facName() << " is tocking {";

  processOutgoingMaterial();

  LOG(LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::makeRequest()
{
  double amt = inventory_.space();
  double min_amt = 0;
  string commodity = in_commodity();

  if (amt > EPS_KG)
    {
      LOG(LEV_INFO4, "EnrFac") << " requests "<< amt << " kg of " << commodity << ".";

      MarketModel* market = MarketModel::marketForCommod(commodity);
      Communicator* recipient = dynamic_cast<Communicator*>(market);

      // create a material resource
      // @MJGFlag note that this doesn't matter in the current state
      mat_rsrc_ptr request_res = 
        mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(in_recipe_)));
      request_res->setQuantity(amt);
      
      // build the transaction and message
      Transaction trans(this, REQUEST);
      trans.setCommod(commodity);
      trans.setMinFrac(min_amt/amt);
      trans.setPrice(commodity_price_);
      trans.setResource(request_res);

      msg_ptr request(new Message(this, recipient, trans)); 
      request->sendOn();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Transaction EnrichmentFacility::buildTransaction() 
{
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  mat_rsrc_ptr offer_res = mat_rsrc_ptr(new Material());
  offer_res->setQuantity(offer_amt);
  Transaction trans(this, OFFER);

  trans.setCommod(out_commodity());
  trans.setMinFrac(min_amt/offer_amt);
  trans.setPrice(commodity_price());
  trans.setResource(offer_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::makeOffer() 
{
  MarketModel* market = MarketModel::marketForCommod(out_commodity());

  Communicator* recipient = dynamic_cast<Communicator*>(market);


  // note that this is a hack. the amount of the resource being offered
  // is greater than the possible amount that can be serviced
  Transaction trans = buildTransaction();

  msg_ptr msg(new Message(this, recipient, trans)); 

  if (trans.resource()->quantity() > 0) 
    {
      LOG(LEV_INFO4, "EnrFac") << "offers "<< trans.resource()->quantity() << " kg of "
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
      Transaction trans = orders_.front()->trans();

      rsrc_ptr prsrc = trans.resource();
      if (!Material::isMaterial(prsrc)) 
       throw CycOverrideException("Can't process a resource as a non-material");

      mat_rsrc_ptr rsrc = dynamic_pointer_cast<Material>(prsrc);

      LOG(LEV_DEBUG1, "EnrFac") << "Processing material: ";
      rsrc->print();

      Assays assays = getAssays(rsrc);
      double product_qty = uranium_qty(rsrc);
      remove_total += feed_qty(product_qty,assays);

      if (remove_total < inventory_.quantity())
        {
          trans.approveTransfer();
        }
      // else
      //   {
      //     throw CycOverrideException("Can't process more than an EnrFac's inventory size");
      //   }

      orders_.pop_front();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
enrichment::Assays EnrichmentFacility::getAssays(mat_rsrc_ptr rsrc)
{
  double product_assay = uranium_assay(rsrc);
  return Assays(feed_assay(),product_assay,tails_assay());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::define_table()
{
  // declare the state id columns and add it to the table
  table_->addField("ID","INTEGER");
  table_->addField("Time","INTEGER");
  table_->addField("Natural_Uranium","REAL");
  table_->addField("SWU","REAL");
  // we've now defined the table
  table_->tableDefined();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::recordEnrichment(double natural_u, double swu)
{
  LOG(LEV_DEBUG1, "EnrFac") << name() << " has enriched a material:";
  LOG(LEV_DEBUG1, "EnrFac") << "  * Amount: " << natural_u;
  LOG(LEV_DEBUG1, "EnrFac") << "  *    SWU: " << swu;

  if ( !table_->defined() ) define_table();

  data an_id(ID()), time_data(TI->time()), 
    nat_u_data(natural_u), swu_data(swu);
  entry id("ID",an_id), time("Time",time_data), 
    natl_u("Natural_Uranium",nat_u_data), swu_req("SWU",swu_data);
  // construct row
  row aRow;
  aRow.push_back(id), aRow.push_back(time), 
    aRow.push_back(natl_u), aRow.push_back(swu_req);
  // add the row
  table_->addRow(aRow);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" Model* constructEnrichmentFacility() 
{
  return new EnrichmentFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructEnrichmentFacility(Model* model) 
{
      delete model;
}
