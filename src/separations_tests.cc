#include "separations.h"

#include <gtest/gtest.h>
#include <sstream>
#include "cyclus.h"

using pyne::nucname::id;
using cyclus::Composition;
using cyclus::CompMap;
using cyclus::Material;
using cyclus::QueryResult;
using cyclus::Cond;
using cyclus::toolkit::MatQuery;

namespace cycamore {

TEST(SeparationsTests, SepMaterial) {
  CompMap comp;
  comp[id("U235")] = 10;
  comp[id("U238")] = 90;
  comp[id("Pu239")] = 1;
  comp[id("Pu240")] = 2;
  comp[id("Am241")] = 3;
  comp[id("Am242")] = 2.8;
  double qty = 100;
  Composition::Ptr c = Composition::CreateFromMass(comp);
  Material::Ptr mat = Material::CreateUntracked(qty, c);

  std::map<int, double> effs;
  effs[id("U")] = .7;
  effs[id("Pu")] = .4;
  effs[id("Am241")] = .4;

  Material::Ptr sep = SepMaterial(effs, mat);
  MatQuery mqorig(mat);
  MatQuery mqsep(sep);

  EXPECT_DOUBLE_EQ(effs[id("U")] * mqorig.mass("U235"), mqsep.mass("U235"));
  EXPECT_DOUBLE_EQ(effs[id("U")] * mqorig.mass("U238"), mqsep.mass("U238"));
  EXPECT_DOUBLE_EQ(effs[id("Pu")] * mqorig.mass("Pu239"), mqsep.mass("Pu239"));
  EXPECT_DOUBLE_EQ(effs[id("Pu")] * mqorig.mass("Pu240"), mqsep.mass("Pu240"));
  EXPECT_DOUBLE_EQ(effs[id("Am241")] * mqorig.mass("Am241"), mqsep.mass("Am241"));
  EXPECT_DOUBLE_EQ(0, mqsep.mass("Am242"));
}

} // namespace cycamore

