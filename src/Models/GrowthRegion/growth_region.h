// growth_region.h
#ifndef GROWTHREGION_H
#define GROWTHREGION_H

#include "region_model.h"
#include "commodity.h"
#include "supply_demand_manager.h"
#include "building_manager.h"
//#include "QueryEngine.h"//

#include <set>

// forward declarations
namespace cycamore {
class GrowthRegion;
} // namespace cycamore

// forward includes
#include "growth_region_tests.h"

namespace cycamore {
/**
   The GrowthRegion class inherits from the RegionModel class and is
   dynamically loaded by the Model class when requested.

   This region determines if there is a need to meet a certain
   capacity (as defined via input) at each time step. If there is
   such a need, the region will determine how many of each facility
   type are required and then determine, facility by facility, which
   of its institutions are available to build each facility.
 */

struct DemandInfo {
  std::string type;
  std::string params;
  int time;
};

class GrowthRegion : public cyclus::RegionModel {
  friend class GrowthRegionTests;
 public:
  /* --- Module Members --- */
  /**
     The default constructor for the GrowthRegion
   */
  GrowthRegion(cyclus::Context* ctx);

  /**
     The default destructor for the GrowthRegion
   */
  virtual ~GrowthRegion();

  virtual std::string schema();

  /* --- */

  /* --- Region Members --- */
  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitFrom(cyclus::QueryEngine* qe);

  /**
     add a demand for a commodity on which this region request that
     facilities be built
   */
  void AddCommodityDemand(cyclus::Commodity commod);

  /**
     perform module-specific tasks when entering the simulation
   */
  virtual void Deploy(cyclus::Model* parent);

  /**
     On each tick, the GrowthRegion queries its supply demand manager
     to determine if there exists some demand. If demand for a
     commodity exists, then the correct build order for that demand
     is constructed and executed.
     @param time is the time to perform the tick
   */
  virtual void Tick(int time);
  /* --- */

  virtual cyclus::Model* Clone() {
    GrowthRegion* m = new GrowthRegion(context());
    m->InitFrom(this);
    return m;
  }

  void InitFrom(GrowthRegion* m);

 protected:
  /* --- GrowthRegion Members --- */
  /// a container of all commodities managed by region
  std::set<cyclus::Commodity, cyclus::CommodityCompare> commodities_;

  std::map<std::string, std::vector<DemandInfo> > demands_;

  /// manager for building things
  cyclus::BuildingManager buildmanager_;

  /// manager for Supply and emand
  cyclus::SupplyDemandManager sdmanager_;

  /**
     register a commodity for which production capacity is being
     demanded region
     @param commodity a reference to the commodity
   */
  void RegisterCommodity(cyclus::Commodity& commodity);

  /**
     register a child as a commodity producer manager if it is one
     @param model the child to register
   */
  void RegisterCommodityProducerManager(cyclus::Model* model);

  /**
     register a child as a builder if it is one
     @param model the child to register
   */
  void RegisterBuilder(cyclus::Model* model);

  /**
     orders builds given a commodity and an unmet demand for production
     capacity of that commodity
     @param commodity the commodity being demanded
     @param unmetdemand the unmet demand
   */
  void OrderBuilds(cyclus::Commodity& commodity, double unmetdemand);

  /**
     orders builder to build a prototype
     @param builder the model that can build buildee
     @param prototype the model to be built
   */
  void OrderBuild(cyclus::Model* builder, cyclus::Model* prototype);
  /* --- */
};
} // namespace cycamore
#endif
