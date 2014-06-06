#ifndef CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_REGION_H_
#define CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_REGION_H_

#include <string>

#include "cyclus.h"

namespace commodconverter {

/// @class CommodconverterRegion
///
/// This Region is intended
/// as a skeleton to guide the implementation of new Region agents.
///
/// The CommodconverterRegion class inherits from the Region class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section intro Introduction
/// Place an introduction to the agent here.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///
/// @section optionalparams Optional Parameters
/// Place a description of the optional input parameters to define the
/// agent implementation.
///
/// @section detailed Detailed Behavior
/// Place a description of the detailed behavior of the agent. Consider
/// describing the behavior at the tick and tock as well as the behavior
/// upon sending and receiving materials and messages.
class CommodconverterRegion : public cyclus::Region {
 public:
  /// Constructor for CommodconverterRegion Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit CommodconverterRegion(cyclus::Context* ctx);

  /// every agent should be destructable
  virtual ~CommodconverterRegion();

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus

  #pragma cyclus note {"doc": "A commodconverter region is provided as a skeleton " \
                              "for the design of new region agents."}

  /// A verbose printer for the CommodconverterRegion
  virtual std::string str();

  // and away we go!
};

}  // namespace commodconverter

#endif  // CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_REGION_H_
