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
using namespace Enrichment;

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

  data = qe->getElementContent("tails_assay");
  set_tails_assay(lexical_cast<double>(data));

  set_feed_assay(uranium_assay(mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(in_recipe_)))));
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
  setMaxInventorySize(source->maxInventorySize() );
  set_commodity_price(source->commodity_price());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTick(int time)
{
  LOG(LEV_INFO3, "EnrFac") << facName() << " is tocking {";

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
void EnrichmentFacility::addResource(Transaction trans, std::vector<rsrc_ptr> manifest) 
{
  inventory_.pushAll(MatBuff::toMat(manifest));
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

  gen_rsrc_ptr offer_res = 
    gen_rsrc_ptr(new GenericResource(out_commodity(),"kg",offer_amt));

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
  msg->sendOn();
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

      Assays assays = getAssays(rsrc);
      double product_qty = uranium_qty(rsrc);
      remove_total += feed_qty(product_qty,assays);

      if (remove_total > inventory_.quantity())
        throw CycOverrideException("Can't process more than an EnrFac's inventory size");

      trans.approveTransfer();
      orders_.pop_front();
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Enrichment::Assays EnrichmentFacility::getAssays(mat_rsrc_ptr rsrc)
{
  double product_assay = uranium_assay(rsrc);
  return Assays(feed_assay(),product_assay,tails_assay());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::receiveMessage(msg_ptr msg)
{
  // is this a message from on high? 
  if(msg->trans().supplier() == this)
    {
      // file the order
      orders_.push_back(msg);
      LOG(LEV_INFO5, "EnrFac") << name() << " just received an order.";
    } 
  else 
    {
      throw CycException("EnrFacility is not the supplier of this msg.");
    }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::vector<rsrc_ptr> EnrichmentFacility::removeResource(Transaction order) 
{
  rsrc_ptr prsrc = order.resource();
  if (!Material::isMaterial(prsrc)) 
    throw CycOverrideException("Can't process a resource as a non-material");
  
  mat_rsrc_ptr rsrc = dynamic_pointer_cast<Material>(prsrc);

  Assays assays = getAssays(rsrc);
  double product_qty = uranium_qty(rsrc);
  double swu = swu_required(product_qty,assays);
  double natural_u = feed_qty(product_qty,assays);
  inventory_.popQty(natural_u);
  recordEnrichment(natural_u,swu);

  vector<rsrc_ptr> ret;
  ret.push_back(order.resource());
  return ret;
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
