#ifndef CYCAMORE_SRC_GROWTH_REGION_H_
#define CYCAMORE_SRC_GROWTH_REGION_H_

#include <set>
#include <vector>

#include "cyclus.h"


// forward declarations
namespace cycamore {
class GrowthRegion;
}  // namespace cycamore

// forward includes
#include "growth_region_tests.h"

namespace cycamore {
/// This region determines if there is a need to meet a certain
/// capacity (as defined via input) at each time step. If there is
/// such a need, the region will determine how many of each facility
/// type are required and then determine, facility by facility, which
/// of its institutions are available to build each facility.
///
/// @TODO In order to make GrowthRegion copacetic with init/restart, its input
/// parameter space was simplified. For now it can only provide growth support
/// for a single demanded commodity. A relatively simple next step will be to
/// determin an input API and corresponding implementation that again supports
/// multiple commodities being demanded.
///
/// @warning The growth region is experimental
class GrowthRegion : public cyclus::Region {
  friend class GrowthRegionTests;
 public:
  /// The default constructor for the GrowthRegion
  GrowthRegion(cyclus::Context* ctx);

  /// The default destructor for the GrowthRegion
  virtual ~GrowthRegion();

  #pragma cyclus

  #pragma cyclus note {"doc": "A region that governs a scenario in which " \
                              "there is growth in demand for a commodity. "}

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
  virtual void Tick();

  /// enter the simulation and register any children present
  virtual void EnterNotify();

  /// register a new child
  virtual void BuildNotify(Agent* m);

  /// unregister a child
  virtual void DecomNotify(Agent* m);

  inline cyclus::toolkit::SupplyDemandManager* sdmanager() {
    return &sdmanager_;
  }

 protected:
  #pragma cyclus var {"tooltip": "commodity in demand", \
                      "doc": "name of the commodity experiencing a " \
                             "growth in demand", \
                      "uitype": "commodity"}
  std::string commodity_name;

  #pragma cyclus var {"tooltip": "demand type", \
                      "doc": "mathematical description of demand growth " \
                             "(i.e., linear, exponential, piecewise)"}
  std::vector<std::string> demand_types;

  #pragma cyclus var {"tooltip": "demand parameters", \
                      "doc": "parameters that define the behavior of the " \
                             "demand type function"}
  std::vector<std::string> demand_params;

  #pragma cyclus var {"tooltip": "demand times", \
                      "doc": "vector describing the length of times " \
                             "regarding the piecewise demand type"}
  std::vector<int> demand_times;

  cyclus::toolkit::Commodity commod_;

  /// manager for building things
  cyclus::toolkit::BuildingManager buildmanager_;

  /// manager for Supply and demand
  cyclus::toolkit::SupplyDemandManager sdmanager_;

  /// register a child
  void Register_(cyclus::Agent* agent);

  /// unregister a child
  void Unregister_(cyclus::Agent* agent);

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

}  // namespace cycamore

#endif  // CYCAMORE_SRC_GROWTH_REGION_H_
