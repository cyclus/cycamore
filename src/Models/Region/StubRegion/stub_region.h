// stub_region.h
#if !defined(_STUBREGION_H)
#define _STUBREGION_H

#include "region_model.h"

/**
   @class StubRegion

   This RegionModel is intended
   as a skeleton to guide the implementation of new RegionModel models.

   The StubRegion class inherits from the RegionModel class and is
   dynamically loaded by the Model class when requested.

   @section intro Introduction
   Place an introduction to the model here.

   @section modelparams Model Parameters
   Place a description of the required input parameters which define the
   model implementation.

   @section optionalparams Optional Parameters
   Place a description of the optional input parameters to define the
   model implementation.

   @section detailed Detailed Behavior
   Place a description of the detailed behavior of the model. Consider
   describing the behavior at the tick and tock as well as the behavior
   upon sending and receiving materials and messages.
 */
class StubRegion : public cyclus::RegionModel {
  /* --------------------
   * all MODEL classes have these members
   * --------------------
   */
 public:
  /**
     Default constructor for StubStub Class
   */
  StubRegion();

  /**
     every model should be destructable
   */
  virtual ~StubRegion();

  /**
     every model should be able to print a verbose description
   */
  virtual std::string str();

  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void InitModuleMembers(cyclus::QueryEngine* qe);

  /**
     perform all necessary actions for the model to enter the simulation
   */
  virtual void EnterSimulation(cyclus::Model* parent);

  /* ------------------- */


  /* --------------------
   * all COMMUNICATOR classes have these members
   * --------------------
   */
 public:
  /**
     The StubRegion should ignore incoming messages
   */
  virtual void ReceiveMessage(cyclus::Message::Ptr msg);

  /* -------------------- */


  /* --------------------
   * all REGIONMODEL classes have these members
   * --------------------
   */

  /* ------------------- */

};

#endif
