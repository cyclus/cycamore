#include "mixing_fab.h"

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
  namespace mixingfabtests {
    
    
    // multiple fissile streams can be correctly requested and used as
    // fissile material inventory.
    TEST(MixingFabTests, MultipleFissStreams) {
         }
    
    // fissile stream preferences can be specified.
    TEST(MixingFabTests, FissStreamPrefs) {
         }
    
    // zero throughput must not result in a zero capacity constraint excception.
    TEST(MixingFabTests, ZeroThroughput) {
         }
    
    // fill, fiss, and topup inventories are all requested for and
    // filled as expected. Inventory size constraints are properly
    // enforced after they are full.
    TEST(MixingFabTests, FillAllInventories) {
         }
    
    // Meet a request requiring zero fill inventory when we have zero fill
    // inventory quantity.
    TEST(MixingFabTests, ProvideStraightFiss_WithZeroFill) {
          }
    
    TEST(MixingFabTests, ProvideStraightFill_ZeroFiss) {
         }
    
    // throughput is properly restricted when faced with many fuel
    // requests and with ample material inventory.
    TEST(MixingFabTests, ThroughputLimit) {
        }
    
    // supplied fuel has proper equivalence weights as requested.
    TEST(MixingFabTests, CorrectMixing) {
     
    }
    
    // fuel is requested requiring more filler than is available with plenty of
    // fissile.
    TEST(MixingFabTests, FillConstrained) {
   
    }
    
    // fuel is requested requiring more fissile material than is available with
    // plenty of filler.
    TEST(MixingFabTests, FissConstrained) {
   
    }
    
    // swap to topup inventory because fissile has too low reactivity.
    TEST(MixingFabTests, SwapTopup) {
    }
    
    TEST(MixingFabTests, SwapTopup_ZeroFill) {
    }
    
    // swap to topup inventory but are limited by topup inventory quantity.  This
    // test makes sure the provided fuel is much less than requested due to a
    // small topup inventory despite a surplus of fill that can't be used due to
    // the fiss stream not having a high enough weight (so we must use topup with
    // fiss).
    TEST(MixingFabTests, SwapTopup_TopupConstrained) {
    }
    
    // swap to topup inventory but are limited by fiss inventory quantity.  This
    // test makes sure the provided fuel is much less than requested due to a
    // small fiss inventory.
    TEST(MixingFabTests, SwapTopup_FissConstrained) {
    }
    
    // Before this test and a fix, the fuel fab (partially) assumed each entire material
    // buffer had the same composition as the material on top of the buffer when
    // calculating stream mixing ratios for material to supply.  This problem was
    // compounded by the fact that material weights are computed on an atom basis
    // and mixing is done on a mass basis - corresponding conversions resulted in
    // the fab being matched for more than it could actually supply - due to
    // thinking it had an inventory of higher quality material than was actually
    // the case.  This test makes sure that doesn't happen again.
    TEST(MixingFabTests, HomogenousBuffers) {
    }
    
  } // namespace MixingFabtests
} // namespace cycamore


