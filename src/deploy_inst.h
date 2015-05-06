#ifndef CYCAMORE_SRC_DEPLOY_INST_H_
#define CYCAMORE_SRC_DEPLOY_INST_H_

#include <utility>
#include <set>
#include <map>

#include "cyclus.h"

namespace cycamore {

typedef std::map<int, std::vector<std::string> > BuildSched;

// Builds and manages agents (facilities) according to a manually specified
// deployment schedule. Deployed agents are automatically decommissioned at
// the end of their lifetime.  The user specifies a list of prototypes for
// each and corresponding build times, number to build, and (optionally)
// lifetimes.  The same prototype can be specified multiple times with any
// combination of the same or different build times, build number, and
// lifetimes.
class DeployInst : public cyclus::Institution {
  #pragma cyclus note { \
    "doc": \
      "Builds and manages agents (facilities) according to a manually specified" \
      " deployment schedule. Deployed agents are automatically decommissioned at" \
      " the end of their lifetime.  The user specifies a list of prototypes for" \
      " each and corresponding build times, number to build, and (optionally)" \
      " lifetimes.  The same prototype can be specified multiple times with any" \
      " combination of the same or different build times, build number, and" \
      " lifetimes. " \
  }
 public:
  DeployInst(cyclus::Context* ctx);

  virtual ~DeployInst();

  #pragma cyclus

  virtual void Build(cyclus::Agent* parent);

  virtual void EnterNotify();

 protected:
  #pragma cyclus var { \
    "doc": "Ordered list of prototypes to build.", \
    "uitype": ("oneormore", "prototype"), \
  }
  std::vector<std::string> prototypes;

  #pragma cyclus var { \
    "doc": "Time step on which to build agents given in prototypes (same order).", \
  }
  std::vector<int> build_times;

  #pragma cyclus var { \
    "doc": "Number of each prototype in prototypes var to build (same order).", \
  }
  std::vector<int> n_build;

  #pragma cyclus var { \
    "doc": "Lifetimes for each prototype in protos (same order)." \
           " These lifetimes override the lifetimes in the original prototype definition." \
           " If unspecified, lifetimes from the original prototype definitions are used." \
           " Although a new prototype is created in the Prototypes table for each lifetime with the suffix '_life_[lifetime]'," \
           " all deployed agents themselves will have the same original prototype name (and so will the Agents tables).", \
    "default": [], \
  }
  std::vector<int> lifetimes;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_DEPLOY_INST_H_
