// Deployinst.h
#ifndef _DEPLOYINST_H
#define _DEPLOYINST_H

#include "cyclus.h"

#include <utility>
#include <set>
#include <map>

namespace cycamore {

typedef std::map<int, std::vector<std::string> > BuildSched;

///  @class DeployInst
///  The DeployInst class inherits from the Institution
///  class and is dynamically loaded by the Agent class when requested.
///
///  This agent implements a simple institution agent that deploys
///  specific facilities as defined explicitly in the input file.
class DeployInst : public cyclus::Institution {
 public:
  DeployInst(cyclus::Context* ctx);

  virtual ~DeployInst();

  #pragma cyclus decl clone

  #pragma cyclus decl schema

  #pragma cyclus decl infiletodb

  #pragma cyclus decl initfromdb

  #pragma cyclus decl initfromcopy

  #pragma cyclus decl snapshot

///  #pragma cyclus decl annotations

  #pragma cyclus note {"doc": "An institution that owns, operates, and " \
                              "deploys facilities manually defined in " \
                              "the input file."}
  void Build(cyclus::Agent* parent);

 protected:
///     a collection of orders to build
  BuildSched build_sched_;
};

} // namespace cycamore
#endif
