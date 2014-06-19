#ifndef CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
#define CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_

#include <string>

#include "cyclus.h"

// forward declaration
namespace commodconverter {
class CommodconverterFacility;
} // namespace commodconverter


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
class CommodconverterFacility 
  : public cyclus::Facility,
    public cyclus::toolkit::CommodityProducer {
 public:  
  /// Constructor for CommodconverterFacility Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  CommodconverterFacility(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A commodconverter facility converts from one " \
                              "commodity to another, with an optional delay."}

  /// A verbose printer for the CommodconverterFacility
  virtual std::string str();
  
  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the CommodconverterFacility.
  /// @param time the time of the tock
  virtual void Tock();

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodity accepted by this facility"}
  std::string in_commod_;

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility"}
  std::string out_commod_;

  #pragma cyclus var {"tooltip":"input recipe",\
                      "doc":"recipe accpeted by this facility"}
  std::string in_recipe_;

  #pragma cyclus var {"tooltip":"output recipe",\
                      "doc":"recipe produced by this facility"}
  std::string out_recipe_;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time",\
                      "doc":"the time it takes to convert a received commodity."}
  int process_time_; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time."}
  double max_inv_size_; //should be nonnegative

  #pragma cyclus var{'capacity': 'max_inv_size_'}

  /// @brief the processing time required for a full process
  inline void process_time(int t) { process_time_ = t; }
  inline int process_time() const { return process_time_; }

  /// @brief the maximum amount in processing at a single time
  inline void capacity(double c) { max_inv_size_ = c; }
  inline int capacity() const { return max_inv_size_; }

  friend class CommodconverterFacilityTest;
};

}  // namespace commodconverter

#endif  // CYCLUS_COMMODCONVERTERS_COMMODCONVERTER_FACILITY_H_
