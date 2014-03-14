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
   The DeployInst class inherits from the InstModel
   class and is dynamically loaded by the Model class when requested.

   This model implements a simple institution model that deploys
   specific facilities as defined explicitly in the input file.
 */
class DeployInst : public cyc::InstModel {
 public:
  DeployInst(cyc::Context* ctx);

  virtual ~DeployInst();

  virtual std::string schema();

  virtual cyc::Model* Clone() {
    DeployInst* m = new DeployInst(context());
    m->InitFrom(this);
    return m;
  }

  /**
     initialize members from a different model
  */
  void InitFrom(DeployInst* m) {
    cyc::InstModel::InitFrom(m);
    build_sched_ = m->build_sched_;
  };

  void Build(cyc::Model* parent);

  virtual void InfileToDb(cyc::QueryEngine* qe, cyc::DbInit di);

  virtual void InitFrom(cyc::QueryBackend* b);

  virtual void Snapshot(cyc::DbInit di);

 protected:
  /**
     a collection of orders to build
   */
  BuildSched build_sched_;

};
} // namespace cycamore
#endif
