// GrowthRegion.h
#ifndef GROWTHREGION_H
#define GROWTHREGION_H

#include "RegionModel.h"
#include "Commodity.h"
#include "SupplyDemandManager.h"
#include "BuildingManager.h"
//#include "QueryEngine.h"//

#include <set>

// forward declarations
class cyclus::QueryEngine;
class GrowthRegion;

// forward includes
#include "GrowthRegionTests.h"

/**
   The GrowthRegion class inherits from the RegionModel class and is 
   dynamically loaded by the Model class when requested. 
    
   This region determines if there is a need to meet a certain 
   capacity (as defined via input) at each time step. If there is 
   such a need, the region will determine how many of each facility 
   type are required and then determine, facility by facility, which 
   of its institutions are available to build each facility. 
 */
class GrowthRegion : public cyclus::RegionModel  
{  
  friend class GrowthRegionTests;
 public:
  /* --- Module Members --- */
  /**
     The default constructor for the GrowthRegion 
   */
  GrowthRegion();

  /**
     The default destructor for the GrowthRegion 
   */
  virtual ~GrowthRegion();
  /* --- */

  /* --- Region Members --- */
  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(cyclus::QueryEngine* qe);

  /**
     add a demand for a commodity on which this region request that
     facilities be built
   */
  void addCommodityDemand(cyclus::QueryEngine* qe);

  /**
     perform module-specific tasks when entering the simulation
   */
  virtual void enterSimulationAsModule();

  /**
     On each tick, the GrowthRegion queries its supply demand manager
     to determine if there exists some demand. If demand for a 
     commodity exists, then the correct build order for that demand
     is constructed and executed.
     @param time is the time to perform the tick 
   */
  virtual void handleTick(int time);
  /* --- */

 protected:
  /* --- GrowthRegion Members --- */
  /// a container of all commodities managed by region
  std::set<cyclus::Commodity,cyclus::CommodityCompare> commodities_;

  /// manager for supply and demand
  cyclus::SupplyDemand::SupplyDemandManager sdmanager_;

  /// manager for building things
  cyclus::ActionBuilding::BuildingManager buildmanager_;

  /**
     register a commodity for which production capacity is being 
     demanded region
     @param commodity a reference to the commodity
   */
  void registerCommodity(cyclus::Commodity& commodity);

  /**
     register a child as a commodity producer manager if it is one
     @param model the child to register
   */
  void registerCommodityProducerManager(cyclus::Model* model);

  /**
     register a child as a builder if it is one
     @param model the child to register
   */
  void registerBuilder(cyclus::Model* model);

  /**
     orders builds given a commodity and an unmet demand for production
     capacity of that commodity
     @param commodity the commodity being demanded
     @param unmet_demand the unmet demand
   */
  void orderBuilds(cyclus::Commodity& commodity, double unmet_demand);

  /**
     orders builder to build a prototype
     @param builder the model that can build buildee
     @param prototype the model to be built
   */
  void orderBuild(cyclus::Model* builder, cyclus::Model* prototype);
  /* --- */
};

#endif
