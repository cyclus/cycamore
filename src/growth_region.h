// growth_region.h
#ifndef CYCAMORE_SRC_GROWTH_REGION_H_
#define CYCAMORE_SRC_GROWTH_REGION_H_

#include <set>
#include <vector>

#include "cyclus.h"


// forward declarations
namespace cycamore {
class GrowthRegion;
} // namespace cycamore

// forward includes
#include "growth_region_tests.h"

namespace cycamore {
/// The GrowthRegion class inherits from the Region class and is
/// dynamically loaded by the Agent class when requested.
///
/// This region determines if there is a need to meet a certain
/// capacity (as defined via input) at each time step. If there is
/// such a need, the region will determine how many of each facility
/// type are required and then determine, facility by facility, which
/// of its institutions are available to build each facility.
struct DemandInfo {
  std::string type;
  std::string params;
  int time;
};


/// @warning The growth region is experimental
class GrowthRegion : public cyclus::Region {
  friend class GrowthRegionTests;
 public:
  /// The default constructor for the GrowthRegion
  GrowthRegion(cyclus::Context* ctx);

  /// The default destructor for the GrowthRegion
  virtual ~GrowthRegion();

  #pragma cyclus

  /// add a demand for a commodity on which this region request that
  /// facilities be built
  void AddCommodityDemand(cyclus::toolkit::Commodity commod);

  /// perform module-specific tasks when entering the simulation
  virtual void Build(cyclus::Agent* parent);

  /// On each tick, the GrowthRegion queries its supply demand manager
  /// to determine if there exists some demand. If demand for a
  /// commodity exists, then the correct build order for that demand
  /// is constructed and executed.
  /// @param time is the time to perform the tick
  virtual void Tick(int time);

  virtual void BuildNotify(Agent* m);

  inline cyclus::toolkit::SupplyDemandManager sdmanager() { return sdmanager_; };

 protected:
  /// a container of all commodities managed by region
  std::set<cyclus::toolkit::Commodity, cyclus::toolkit::CommodityCompare> commodities_;

  std::map<std::string, std::vector<DemandInfo> > demands_;

  /// manager for building things
  cyclus::toolkit::BuildingManager buildmanager_;

  /// manager for Supply and demand
  cyclus::toolkit::SupplyDemandManager sdmanager_;

  /// register a child as a commodity producer manager if it is one
  /// @param agent the child to register
  void RegisterCommodityProducerManager(cyclus::Agent* agent);

  /// register a child as a builder if it is one
  /// @param agent the child to register
  void RegisterBuilder(cyclus::Agent* agent);

  /// orders builds given a commodity and an unmet demand for production
  /// capacity of that commodity
  /// @param commodity the commodity being demanded
  /// @param unmetdemand the unmet demand
  void OrderBuilds(cyclus::toolkit::Commodity& commodity, double unmetdemand);

  /// orders builder to build a prototype
  /// @param builder the agent that can build buildee
  /// @param prototype the agent to be built
  void OrderBuild(cyclus::Agent* builder, cyclus::Agent* prototype);
};
} // namespace cycamore

#endif  // CYCAMORE_SRC_GROWTH_REGION_H_
