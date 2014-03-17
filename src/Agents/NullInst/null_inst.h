// null_inst.h
#ifndef CYCAMORE_SRC_NULL_INST_H_
#define CYCAMORE_SRC_NULL_INST_H_

#include "inst_model.h"

namespace cycamore {

/**
   @class NullInst

   @section introduction Introduction
   The NullInst is an institution type in Cyclus which performs the
   most basic institution functionality as defined by the InstAgent
   class.

   @section detailedBehavior Detailed Behavior
   The NullInst starts operation at the beginning of the
   simulation and ends operation at the end of the simulation. It
   populates its list of facilities when it is initialized and
   determines its region as all Institution models do. When it
   receives a message, it transmits that message immediately up to
   its region or down to the appropriate facility without making
   any changes.
 */

class NullInst : public cyclus::InstAgent {
  /* --------------------
   * all MODEL classes have these members
   * --------------------
   */
 public:
  /**
     Default constructor
   */
  NullInst(cyclus::Context* ctx);

  /**
     Default destructor
   */
  virtual ~NullInst();

  virtual cyclus::Agent* Clone() {
    NullInst* m = new NullInst(context());
    m->InitFrom(this);
    return m;
  }

  void InitFrom(NullInst* m) {
    cyclus::InstAgent::InitFrom(m);
  }

  /* ------------------- */
};
}  // namespace cycamore

#endif  // CYCAMORE_SRC_NULL_INST_H_
