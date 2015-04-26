#include "source.h"

#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

namespace cycamore {

Source::Source(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      throughput(std::numeric_limits<double>::max()),
      inventory_size(std::numeric_limits<double>::max()),
      outrecipe("") {}

Source::~Source() {}

void Source::InitFrom(Source* m) {
  #pragma cyclus impl initfromcopy cycamore::Source
  cyclus::toolkit::CommodityProducer::Copy(m);
}

void Source::InitFrom(cyclus::QueryableBackend* b) {
  #pragma cyclus impl initfromdb cycamore::Source
  namespace tk = cyclus::toolkit;
  tk::CommodityProducer::Add(tk::Commodity(outcommod),
                             tk::CommodInfo(throughput, throughput));
}

void Source::EnterNotify() {
  cyclus::Facility::EnterNotify();

  cyclus::Composition::Ptr c;
  bool ignore_comp = outrecipe.empty();
  if (!ignore_comp) {
    c = context()->GetRecipe(outrecipe);
  } else {
    cyclus::CompMap v;
    v[pyne::nucname::id("H1")] = 1;
    c = cyclus::Composition::CreateFromAtom(v);
  }
  inventory.Push(cyclus::Material::Create(this, inventory_size, c));
  sellpol_.Init(this, &inventory, std::string("inv"), throughput,
                ignore_comp, -1);
  sellpol_.Set(outcommod);
  sellpol_.Start();
}

std::string Source::str() {
  namespace tk = cyclus::toolkit;
  std::stringstream ss;
  std::string ans;
  if (cyclus::toolkit::CommodityProducer::Produces(
          cyclus::toolkit::Commodity(outcommod))) {
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str() << " supplies commodity '" << outcommod
     << "' with recipe '" << outrecipe << "' at a throughput of "
     << throughput << " kg per time step "
     << " commod producer members: "
     << " produces " << outcommod << "?: " << ans
     << " throughput: " << cyclus::toolkit::CommodityProducer::Capacity(outcommod)
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(outcommod);
  return ss.str();
}

extern "C" cyclus::Agent* ConstructSource(cyclus::Context* ctx) {
  return new Source(ctx);
}

}  // namespace cycamore
