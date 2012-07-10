// GrowthRegion.h
#ifndef GROWTHREGION_H
#define GROWTHREGION_H

#include "RegionModel.h"
#include "Model.h"
#include "SupplyDemand.h"
#include "BuildingManager.h"

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

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
     Initalize the GrowthRegion from xml. Calls the init function. 
     
     @param cur the curren xml node pointer 
   */
  virtual void init(xmlNodePtr cur);

  /**
     initialize an object by copying another 
   */
  virtual void copy(GrowthRegion* src);

  /**
     This drills down the dependency tree to initialize all relevant 
     parameters/containers. 
     Note that this function must be defined only in the specific 
     model in question and not in any inherited models preceding it. 
      
     @param src the pointer to the original (initialized ?) model to be 
   */
  virtual void copyFreshModel(Model* src) { 
    copy(dynamic_cast<GrowthRegion*>(src)); 
  }
  
  /**
     print information about the region 
   */
  virtual std::string str();

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
     @param nodes the xml node corresponding to each demanded
     commodity
   */
  void initCommodity(xmlNodePtr& node);

  /**
     populates builders_ and producers_ once all initialization is 
     complete
   */
  void populateProducerMaps();

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

};

#endif
