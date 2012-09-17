// GrowthRegion.h
#ifndef GROWTHREGION_H
#define GROWTHREGION_H

#include "SupplyDemandManager.h"
#include "RegionModel.h"

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

// forward declarations
class QueryEngine;
class Producer;
class BuildingManager;
class BuildOrder;
class Commodity; 
class GrowthRegion;
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
class GrowthRegion : public RegionModel  
{
/* --------------------
 * all MODEL classes have these members
 * --------------------
 */
 public:
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

  /**
     Initialize members related to derived module class
     @param qe a pointer to a QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(QueryEngine* qe);

  /**
     perform all necessary actions for the model to enter the simulation
   */
  virtual void enterSimulation(Model* parent);

/* ------------------- */ 


/* --------------------
 * all COMMUNICATOR classes have these members
 * --------------------
 */
 public:
  /**
     On each tick, the GrowthRegion queries its supply demand manager
     to determine if there exists some demand. If demand for a 
     commodity exists, then the building manager is queried to
     determine which prototypes to build, and orderBuilds() is called.

     @param time is the time to perform the tick 
   */
  virtual void handleTick(int time);

/* -------------------- */


/* --------------------
 * the GrowthRegion class has these members
 * --------------------
 */
 protected:
  /// a container of all commodities managed by region
  std::vector<Commodity> commodities_;

  /// manager for supply and demand
  SupplyDemandManager sdmanager_;

  /// manager for building things
  boost::shared_ptr<BuildingManager> buildmanager_;

  /// a map for the institutions that can build a prototype
  std::map<Producer*,Model*> builders_;

  /// a map for the prototypes that correspond to supplydemand's producer
  std::map<Producer*,Model*> producers_;

  /**
     initializes members based on commodity demand input
     @param qe the engine to query input
   */
  void initCommodity(QueryEngine* qe);

  /**
     initializes the building manager with a fully formed sdmanager_
   */
  void initBuildManager();

  /**
     initializes members based on producer input
     @param qe the engine to query input
     @param commodity the commodity produced
   */
  Producer getProducer(QueryEngine* qe, Commodity& commodity);

  /**
     populates builders_ and producers_ once all initialization is 
     complete
   */
  void populateProducerMaps();

  /**
     populates producer_names with info from the sdmanager
   */
  void populateProducerNames(Commodity& c, 
                             std::map<std::string,Producer*>& 
                             producer_names);

  /**
     recursively looks to see if the current node is in the map
     of producer names. if so, it will add that model to the 
     appropriate map. in either case, the search continues to that
     node's children.
     @param node the current model node being investigated
     @param producer_names a map of producer's names to thier pointer
   */
  void populateMaps(Model* node, 
                    std::map<std::string,Producer*>& producer_names);

  /**
     provides a string of information about the maps
     @return info about the maps
  */
  std::string printMaps();

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
  
  /* ------------------- */ 
  friend class GrowthRegionTest;
};

#endif
