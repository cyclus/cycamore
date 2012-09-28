// GrowthRegion.h
#ifndef GROWTHREGION_H
#define GROWTHREGION_H

#include "RegionModel.h"
#include "SupplyDemandManager.h"

/**
   The GrowthRegion class inherits from the RegionModel class and is 
   dynamically loaded by the Model class when requested. 
    
   This region determines if there is a need to meet a certain 
   capacity (as defined via input) at each time step. If there is 
   such a need, the region will determine how many of each facility 
   type are required and then determine, facility by facility, which 
   of its institutions are available to build each facility. 
 */
class GrowthRegion : public RegionModel  
{
 public:
  /* --- Module Members --- */
  /**
     The default constructor for the GrowthRegion 
   */
  GrowthRegion();

  /**
     The default destructor for the GrowthRegion 
   */
  virtual ~GrowthRegion() {};

  /**
     print information about the region 
   */
  virtual std::string str();
  /* --- */

  /* --- Region Members --- */
  /**
     Initialize members related to derived module class
     @param qe a pointer to a QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(QueryEngine* qe);

  /**
     perform module-specific tasks when entering the simulation
   */
  virtual void enterSimulationAsModule();

  /**
     On each tick, the GrowthRegion queries its supply demand manager
     to determine if there exists some demand. If demand for a 
     commodity exists, then the building manager is queried to
     determine which prototypes to build, and orderBuilds() is called.

     @param time is the time to perform the tick 
   */
  virtual void handleTick(int time);
  /* --- */

 protected:
  /* --- GrowthRegion Members --- */
  /// a container of all commodities managed by region
  std::set<Commodity> commodities_;

  /// manager for supply and demand
  SupplyDemandManager sdmanager_;

  /// manager for building things
  BuildingManager buildmanager_;

  /**
     calls the appropriate orderBuild() functions given some
     build orders
     @param orders the build orders as determined by the building
     manager
   */
  void orderBuilds(std::vector<BuildOrder>& orders);

  /**
     orders builder to build a prototype
     @param builder the model that can build buildee
     @param prototype the model to be built
   */
  void orderBuild(Model* builder, Model* prototype);
  /* --- */
};

#endif
