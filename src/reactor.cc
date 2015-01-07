#include "reactor.h"

namespace cycamore {

Reactor::Reactor(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      n_batches(0),
      assem_size(0),
      integral_assem(0),
      n_assem_core(0),
      n_assem_spent(0),
      n_assem_fresh(0),
      cycle_time(0),
      refuel_time(0),
      cycle_step(0) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Reactor archetype "
                                             "is experimental");
}

} // namespace cycamore

