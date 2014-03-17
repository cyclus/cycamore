// Deployinst.h
#ifndef _DEPLOYINST_H
#define _DEPLOYINST_H

#include "cyclus.h"

#include <utility>
#include <set>
#include <map>

namespace cyc = cyclus;

namespace cycamore {

typedef std::map<int, std::vector<std::string> > BuildSched;

/**
   @class DeployInst
   The DeployInst class inherits from the Institution
   class and is dynamically loaded by the Agent class when requested.

   This agent implements a simple institution agent that deploys
   specific facilities as defined explicitly in the input file.
 */
class DeployInst : public cyc::Institution {
 public:
  DeployInst(cyc::Context* ctx);

  virtual ~DeployInst();

  virtual std::string schema();

  virtual cyc::Agent* Clone() {
    DeployInst* m = new DeployInst(context());
    m->InitFrom(this);
    return m;
  }

  /**
     initialize members from a different agent
  */
  void InitFrom(DeployInst* m) {
    cyc::Institution::InitFrom(m);
    build_sched_ = m->build_sched_;
  };

  void Build(cyc::Agent* parent);

  virtual void InfileToDb(cyc::InfileTree* qe, cyc::DbInit di);

  virtual void InitFrom(cyc::QueryableBackend* b);

  virtual void Snapshot(cyc::DbInit di);

 protected:
  /**
     a collection of orders to build
   */
  BuildSched build_sched_;

};
} // namespace cycamore
#endif
