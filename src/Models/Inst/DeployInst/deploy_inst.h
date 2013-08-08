// Deployinst.h
#ifndef _DEPLOYINST_H
#define _DEPLOYINST_H

#include "inst_model.h"

#include "prototype.h"

#include <utility>
#include <set>
#include <map>

typedef std::pair<cyclus::Prototype*, int> BuildOrder;

/**
   a helper class for storing and extracting build orders
 */
class BuildOrderList {
 public:
  /// add a build order
  void AddBuildOrder(cyclus::Prototype* p, int number, int time);

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
  DeployInst();

  /**
     Destructor
   */
  virtual ~DeployInst();

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitModuleMembers(cyclus::QueryEngine* qe);

  /* ------------------- */

  /* --------------------
   * all INSTMODEL classes have these members
   * --------------------
   */
 public:
  /**
     tick handling function for this inst
   */
  virtual void HandleTick(int time);

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

#endif
