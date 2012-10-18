// EnrichmentFacility.cpp
// Implements the EnrichmentFacility class
#include "EnrichmentFacility.h"

#include "QueryEngine.h"
#include "Logger.h"
#include "CycException.h"
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::EnrichmentFacility()
commod_price_(0) 
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
      set_max_inventory_size(lexical_cast<double>(data));
    }
  catch (CycNullQueryException e) 
    {
      set_max_inventory_size(numeric_limits<double>::max());
    }

  QueryEngine* output = qe->queryElement("output");
  set_out_commodity(output->getElementContent("outcommodity"));

  data = qe->getElementContent("tails_assay");
  set_tails_assay(lexical_cast<double>(data));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::str() 
{
  std::stringstream ss;
  ss << FacilityModel::str()
     << " with enrichment facility parameters:"
     << " * Tails assay: " << tails_assay_;
     << " * Input Commodity: " << in_commodity_;
     << " * Output Commodity: " << out_commodity_;
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::cloneModuleMembersFrom(FacilityModel* sourceModel) 
{
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
  tails_assay_ = source->tails_assay();
  in_commodity_ = source->in_commodity();
  out_commodity_ = source->out_commodity();
  setMaxInventorySize(source->max_inventory_size());
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

      // create a generic resource
      mat_rsrc_ptr request_res = 
        mat_rsrc_ptr(new Material(RecipeLibrary::Recipe(recipe_name_)));
      request_res->setQuantity(amt);
      
      // build the transaction and message
      Transaction trans(this, REQUEST);
      trans.setCommod(commodity);
      trans.setMinFrac(min_amt/amt);
      trans.setPrice(commod_price_);
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

  msg_ptr msg(new Message(this, recipient, buildTransaction())); 
  msg->sendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::processOutgoingMaterial()
{
  // while ! orders empty?
  while (!orders_.empty())
    {
      Transaction order = orders_.front()->trans();
      double natural_u = naturalUraniumRequired(order->trans);
      double swu = swuRequired(order->trans);
      recordEnrichment(natural_u,swu);
      inventory_.popQty(natural_u);
      order.approveTransfer();
      orders_.pop_front();
    }
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
  return vector<rsrc_ptr>();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::sendOffer(Transaction trans) 
{
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
