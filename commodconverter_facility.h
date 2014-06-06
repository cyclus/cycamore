#ifndef CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
#define CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_

#include <string>

#include "cyclus.h"

namespace commodconverter {


/// @class CommodconverterFacility
///
/// This Facility is intended
/// as a skeleton to guide the implementation of new Facility
/// agents.
/// The CommodconverterFacility class inherits from the Facility class and is
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
class CommodconverterFacility : public cyclus::Facility  {
 public:  
  /// Constructor for CommodconverterFacility Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit CommodconverterFacility(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus

  #pragma cyclus note {"doc": "A commodconverter facility is provided as a skeleton " \
                              "for the design of new facility agents."}

  /// A verbose printer for the CommodconverterFacility
  virtual std::string str();
  
  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tock
  virtual void Tock();

  // and away we go!
};

}  // namespace commodconverter

#endif  // CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
