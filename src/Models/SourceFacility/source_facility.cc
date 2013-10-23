// source_facility.cc
// Implements the SourceFacility class
#include "source_facility.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "cyc_limits.h"
#include "context.h"
#include "error.h"
#include "generic_resource.h"
#include "logger.h"
#include "market_model.h"
#include "query_engine.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility(cyclus::Context* ctx)
    : cyclus::FacilityModel(ctx),
      out_commod_(""),
      recipe_name_(""),
      commod_price_(0),
      capacity_(std::numeric_limits<double>::max()) {
  using std::deque;
  using std::numeric_limits;
  ordersWaiting_ = deque<cyclus::Message::Ptr>();
  inventory_ = cyclus::ResourceBuff();
  SetMaxInventorySize(numeric_limits<double>::max());
}

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
    "    <optional>                        \n"
    "      <ref name=\"inventorysize\"/>   \n"
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

  SetRecipe(output->GetElementContent("recipe"));

  string data = output->GetElementContent("outcommodity");
  SetCommodity(data);
  cyclus::Commodity commod(data);
  cyclus::CommodityProducer::AddCommodity(commod);

  double capacity =
      cyclus::GetOptionalQuery<double>(output,
                                       "output_capacity",
                                       numeric_limits<double>::max());
  cyclus::CommodityProducer::SetCapacity(commod, capacity);
  SetCapacity(capacity);

  double size =
      cyclus::GetOptionalQuery<double>(output,
                                       "inventorysize",
                                       numeric_limits<double>::max());
  SetMaxInventorySize(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '"
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step "
     << " with max inventory of " << inventory_.capacity() << " kg.";
  return "" + ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacility::Clone() {
  SourceFacility* m = new SourceFacility(*this);
  m->InitFrom(this);

  m->SetCommodity(commodity());
  m->SetCapacity(capacity());
  m->SetRecipe(recipe());
  m->SetMaxInventorySize(MaxInventorySize());
  m->CopyProducedCommoditiesFrom(this);

  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is ticking {";

  GenerateMaterial();
  cyclus::Transaction trans = BuildTransaction();

  LOG(cyclus::LEV_INFO4, "SrcFac") << "offers " << trans.resource()->quantity() <<
                                   " kg of "
                                   << out_commod_ << ".";

  SendOffer(trans);

  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is tocking {";

  // check what orders are waiting,
  // send material if you have it now
  while (!ordersWaiting_.empty()) {
    cyclus::Transaction order = ordersWaiting_.front()->trans();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Order is for: " <<
                                     order.resource()->quantity();
    LOG(cyclus::LEV_INFO3, "SrcFac") << "Inventory is: " << inventory_.quantity();
    if (order.resource()->quantity() - inventory_.quantity() > cyclus::eps()) {
      LOG(cyclus::LEV_INFO3, "SrcFac") <<
                                       "Not enough inventory. Waitlisting remaining orders.";
      break;
    } else {
      LOG(cyclus::LEV_INFO3, "SrcFac") << "Satisfying order.";
      order.ApproveTransfer();
      ordersWaiting_.pop_front();
    }
  }
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SrcFac") << "SourceFacility " << this->id()
                                   << " is holding " << this->inventory_.quantity()
                                   << " units of material at the close of month " << time
                                   << ".";

  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::ReceiveMessage(cyclus::Message::Ptr msg) {
  if (msg->trans().supplier() == this) {
    // file the order
    ordersWaiting_.push_front(msg);
    LOG(cyclus::LEV_INFO5, "SrcFac") << name() << " just received an order.";
    LOG(cyclus::LEV_INFO5, "SrcFac") << "for " <<
                                     msg->trans().resource()->quantity()
                                     << " of " << msg->trans().commod();
  } else {
    throw cyclus::Error("SourceFacility is not the supplier of this msg.");
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::vector<cyclus::Resource::Ptr> SourceFacility::RemoveResource(
  cyclus::Transaction order) {
  return inventory_.PopQty(order.resource()->quantity());
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetCommodity(std::string name) {
  out_commod_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::commodity() {
  return out_commod_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetCapacity(double capacity) {
  capacity_ = capacity;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::capacity() {
  return capacity_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetRecipe(std::string name) {
  recipe_name_ = name;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::recipe() {
  return recipe_name_;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SetMaxInventorySize(double size) {
  inventory_.set_capacity(size);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::MaxInventorySize() {
  return inventory_.capacity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SourceFacility::InventorySize() {
  return inventory_.quantity();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::GenerateMaterial() {
  using cyclus::Model;
  using cyclus::Material;
  
  double empty_space = inventory_.space();
  if (empty_space < cyclus::eps()) {
    return; // no room
  }

  Material::Ptr newMat;
  double amt = capacity_;
  cyclus::Context* ctx = Model::context();
  if (amt <= empty_space) {
    newMat = Material::Create(this, amt, ctx->GetRecipe(recipe_name_));
  } else {
    newMat = Material::Create(this, empty_space, ctx->GetRecipe(recipe_name_));
  }
  inventory_.Push(newMat);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Transaction SourceFacility::BuildTransaction() {
  using cyclus::Model;
  using cyclus::Material;
  
  // there is no minimum amount a source facility may send
  double min_amt = 0;
  double offer_amt = inventory_.quantity();

  cyclus::Context* ctx = Model::context();
  Material::Ptr trade_res = Material::Create(this,
                                             offer_amt,
                                             ctx->GetRecipe(recipe()));

  cyclus::Transaction trans(this, cyclus::OFFER);
  trans.SetCommod(out_commod_);
  trans.SetMinFrac(min_amt / offer_amt);
  trans.SetPrice(commod_price_);
  trans.SetResource(trade_res);

  return trans;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::SendOffer(cyclus::Transaction trans) {
  cyclus::MarketModel* market = cyclus::MarketModel::MarketForCommod(out_commod_);

  Communicator* recipient = dynamic_cast<Communicator*>(market);

  cyclus::Message::Ptr msg(new cyclus::Message(this, recipient, trans));
  msg->SendOn();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructSourceFacility(cyclus::Context* ctx) {
  return new SourceFacility(ctx);
}
} // namespace cycamore
