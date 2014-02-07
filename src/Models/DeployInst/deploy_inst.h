// Deployinst.h
#ifndef _DEPLOYINST_H
#define _DEPLOYINST_H

#include "inst_model.h"

#include <utility>
#include <set>
#include <map>

namespace cycamore {

typedef std::pair<std::string, int> BuildOrder;

/**
   a helper class for storing and extracting build orders
 */
class BuildOrderList {
 public:
  /// add a build order
  void AddBuildOrder(std::string prototype, int number, int time);

  /// extract a set of build orders
  std::set<BuildOrder> ExtractOrders(int time);

 private:
  std::map<int, std::set<BuildOrder> > all_orders_;
};

/**
   @class DeployInst
   The DeployInst class inherits from the InstModel
   class and is dynamically loaded by the Model class when requested.

   This model implements a simple institution model that deploys
   specific facilities as defined explicitly in the input file.
 */
class DeployInst : public cyclus::InstModel {
  /* --------------------
   * all MODEL classes have these members
   * --------------------
   */
 public:
  /**
     Default constructor
   */
  DeployInst(cyclus::Context* ctx);

  /**
     Destructor
   */
  virtual ~DeployInst();

  virtual std::string schema();

  virtual cyclus::Model* Clone() {
    DeployInst* m = new DeployInst(context());
    m->InitFrom(this);
    return m;
  }

  /**
     initialize members from a different model
  */
  void InitFrom(DeployInst* m) {
    cyclus::InstModel::InitFrom(m);
    build_orders_ = m->build_orders_;
  };

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitFrom(cyclus::QueryEngine* qe);

  /* ------------------- */

  /* --------------------
   * all INSTMODEL classes have these members
   * --------------------
   */
 public:
  /**
     tick handling function for this inst
   */
  virtual void Tick(int time);

  /* ------------------- */

  /* --------------------
   * This INSTMODEL classes have these members
   * --------------------
   */
 protected:
  /**
     a collection of orders to build
   */
  BuildOrderList build_orders_;

  /* ------------------- */

};
} // namespace cycamore
#endif
