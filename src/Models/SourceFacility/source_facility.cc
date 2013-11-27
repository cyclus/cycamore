// source_facility.cc
// Implements the SourceFacility class
#include "source_facility.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

#include "cyc_limits.h"
#include "context.h"
#include "error.h"
#include "exchange_context.h"
#include "logger.h"

namespace cycamore {

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SourceFacility::SourceFacility(cyclus::Context* ctx)
  : cyclus::FacilityModel(ctx),
    cyclus::Model(ctx),
    out_commod_(""),
    recipe_name_(""),
    commod_price_(0),
    capacity_(std::numeric_limits<double>::max()) { }

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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::InitModuleMembers(cyclus::QueryEngine* qe) {
  using std::string;
  using std::numeric_limits;
  using boost::lexical_cast;
  cyclus::QueryEngine* output = qe->QueryElement("output");

  set_recipe(output->GetElementContent("recipe"));

  string data = output->GetElementContent("outcommodity");
  set_commodity(data);
  cyclus::Commodity commod(data);
  cyclus::CommodityProducer::AddCommodity(commod);

  double capacity =
    cyclus::GetOptionalQuery<double>(output,
                                     "output_capacity",
                                     numeric_limits<double>::max());
  cyclus::CommodityProducer::SetCapacity(commod, capacity);
  set_capacity(capacity);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SourceFacility::str() {
  std::stringstream ss;
  ss << cyclus::FacilityModel::str()
     << " supplies commodity '"
     << out_commod_ << "' with recipe '"
     << recipe_name_ << "' at a capacity of "
     << capacity_ << " kg per time step ";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Model* SourceFacility::Clone() {
  SourceFacility* m = new SourceFacility(*this);
  m->InitFrom(this);

  m->set_commodity(commodity());
  m->set_capacity(capacity());
  m->set_recipe(recipe());
  m->CopyProducedCommoditiesFrom(this);

  return m;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTick(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is ticking {";
  LOG(cyclus::LEV_INFO4, "SrcFac") << "will offer " << capacity_
                                   << " kg of "
                                   << out_commod_ << ".";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::HandleTock(int time) {
  LOG(cyclus::LEV_INFO3, "SrcFac") << FacName() << " is tocking {";
  LOG(cyclus::LEV_INFO3, "SrcFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
SourceFacility::AddMatlBids(cyclus::ExchangeContext<cyclus::Material>* ec) {
// Material::Ptr trade_res = Material::CreateUntracked(offer_amt,
//                                                       ctx->GetRecipe(recipe()));  
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SourceFacility::PopulateMatlTradeResponses(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                          cyclus::Material::Ptr> >& responses) {
//     newMat = Material::Create(this, amt, ctx->GetRecipe(recipe_name_));
//     LOG(cyclus::LEV_INFO5, "SrcFac") << name() << " just received an order.";
//     LOG(cyclus::LEV_INFO5, "SrcFac") << "for " <<
//                                      msg->trans().resource()->quantity()
//                                      << " of " << msg->trans().commod();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* ConstructSourceFacility(cyclus::Context* ctx) {
  return new SourceFacility(ctx);
}
} // namespace cycamore
