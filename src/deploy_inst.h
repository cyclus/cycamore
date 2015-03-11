#ifndef CYCAMORE_SRC_DEPLOY_INST_H_
#define CYCAMORE_SRC_DEPLOY_INST_H_

#include <utility>
#include <set>
#include <map>

#include "cyclus.h"

namespace cycamore {

typedef std::map<int, std::vector<std::string> > BuildSched;

///  @class DeployInst
///  The DeployInst class inherits from the Institution
///  class and is dynamically loaded by the Agent class when requested.
///
///  This agent implements a simple institution agent that deploys
///  specific facilities as defined explicitly in the input file.
class DeployInst : public cyclus::Institution {
  #pragma cyclus note {"doc": "An institution that owns, operates, and " \
                              "deploys facilities manually defined in " \
                              "the input file."}
 public:
  DeployInst(cyclus::Context* ctx);

  virtual ~DeployInst();

  #pragma cyclus

  virtual void Build(cyclus::Agent* parent);

 protected:
  #pragma cyclus var { \
    "doc": "Ordered list of prototypes to build.", \
    "uitype": ("onormore", "prototype"), \
  }
  std::vector<std::string> prototypes;

  #pragma cyclus var { \
    "doc": "Number of each prototype in prototypes var to build (same order).", \
    "uitype": ("onormore", "prototype"), \
  }
  std::vector<int> n_build;

  #pragma cyclus var { \
    "doc": "Time step on which to build agents given in prototypes (same order).", \
    "uitype": ("onormore", "prototype"), \
  }
  std::vector<int> build_times;

  #pragma cyclus var { \
    "doc": "Lifetimes for each prototype in protos (same order)." \
           " If unspecified, defaults to the lifetimes as specified in the original prototype definitions." \
           " A new prototype is created for each lifetime with the suffix '_life[lifetime]'.", \
    "default": [], \
    "uitype": "prototype", \
  }
  std::vector<int> lifetimes;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_DEPLOY_INST_H_
