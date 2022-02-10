#ifndef CYCAMORE_SRC_DEPLOY_INST_H_
#define CYCAMORE_SRC_DEPLOY_INST_H_

#include <utility>
#include <set>
#include <map>

#include "cyclus.h"
#include "cycamore_version.h"

namespace cycamore {

typedef std::map<int, std::vector<std::string> > BuildSched;

// Builds and manages agents (facilities) according to a manually specified
// deployment schedule. Deployed agents are automatically decommissioned at
// the end of their lifetime.  The user specifies a list of prototypes for
// each and corresponding build times, number to build, and (optionally)
// lifetimes.  The same prototype can be specified multiple times with any
// combination of the same or different build times, build number, and
// lifetimes.
class DeployInst : 
  public cyclus::Institution, 
  public cyclus::toolkit::CommodityProducerManager,
  public cyclus::toolkit::Position {
  #pragma cyclus note { \
    "doc": \
      "Builds and manages agents (facilities) according to a manually" \
      " specified deployment schedule. Deployed agents are automatically" \
      " decommissioned at the end of their lifetime.  Deployed and" \
      " decommissioned agents are registered and unregistered with a" \
      " region. The user specifies a list of prototypes for" \
      " each and corresponding build times, number to build, and (optionally)" \
      " lifetimes.  The same prototype can be specified multiple times with" \
      " any combination of the same or different build times, build number," \
      " and lifetimes. " \
  }
 public:
  DeployInst(cyclus::Context* ctx);

  virtual ~DeployInst();

  virtual std::string version() { return CYCAMORE_VERSION; }

  #pragma cyclus

  virtual void Build(cyclus::Agent* parent);

  virtual void EnterNotify();

  virtual void BuildNotify(Agent* m);
  virtual void DecomNotify(Agent* m);
  /// write information about a commodity producer to a stream
  /// @param producer the producer
  void WriteProducerInformation(cyclus::toolkit::CommodityProducer*
                                producer);

  private:
  /// register a child
  void Register_(cyclus::Agent* agent);

  /// unregister a child
  void Unregister_(cyclus::Agent* agent);

 protected:
  #pragma cyclus var { \
    "doc": "Ordered list of prototypes to build.", \
    "uitype": ("oneormore", "prototype"), \
    "uilabel": "Prototypes to deploy", \
  }
  std::vector<std::string> prototypes;

  #pragma cyclus var { \
    "doc": "Time step on which to deploy agents given in prototype list " \
           "(same order).",						\
    "uilabel": "Deployment times",					\
  }
  std::vector<int> build_times;

  #pragma cyclus var { \
    "doc": "Number of each prototype given in prototype list that should be " \
           "deployed (same order).", \
    "uilabel": "Number to deploy", \
  }
  std::vector<int> n_build;


  #pragma cyclus var {							\
    "doc": "Lifetimes for each prototype in prototype list (same order)." \
           " These lifetimes override the lifetimes in the original prototype" \
           " definition." \
           " If unspecified, lifetimes from the original prototype definitions"\
           " are used." \
           " Although a new prototype is created in the Prototypes table for" \
           " each lifetime with the suffix '_life_[lifetime]'," \
           " all deployed agents themselves will have the same original" \
           " prototype name (and so will the Agents tables).", \
    "default": [], \
    "uilabel": "Lifetimes" \
  }
  std::vector<int> lifetimes;

 private:
  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double longitude;

  cyclus::toolkit::Position coordinates;

  /// Records an agent's latitude and longitude to the output db
  void RecordPosition();
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_DEPLOY_INST_H_
