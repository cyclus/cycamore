// EnrichmentFacility.h
#ifndef _ENRICHMENTFACILITY_H
#define _ENRICHMENTFACILITY_H

#include "FacilityModel.h"

#include "MatBuff.h"
#include "Transaction.h"
#include "Enrichment.h"

#include <string>
#include <deque>

/**
   @class EnrichmentFacility 
    
   @section introduction Introduction 
 */
class EnrichmentFacility : public cyclus::FacilityModel
{
 public:
  /* --- Module Methods --- */
  /**
     Default Constructor for the EnrichmentFacility class 
   */
  EnrichmentFacility();
  
  /**
     Destructor for the EnrichmentFacility class 
   */
  virtual ~EnrichmentFacility();
 
  /**
     Initialize members related to derived module class
     @param qe a pointer to a cyclus::QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(cyclus::QueryEngine* qe);

  /**
     Print information about this model 
   */
  virtual std::string str();
  /* --- */

  /* --- Facility Methods --- */
  /**
     Copy module members from a source model
     @param sourceModel the model to copy from
   */
  virtual void cloneModuleMembersFrom(cyclus::FacilityModel* sourceModel);
  /* --- */

  /* --- Agent Methods --- */
  /**
     Each facility is prompted to do its beginning-of-time-step 
     stuff at the tick of the timer. 
      
     @param time is the time to perform the tick 
   */
  virtual void handleTick(int time);

  /**
     Each facility is prompted to its end-of-time-step 
     stuff on the tock of the timer. 
      
     @param time is the time to perform the tock 
   */
  virtual void handleTock(int time);
  /* --- */

  /* --- cyclus::Transaction Methods --- */
  /**
     When this facility receives a message, execute the transaction 
   */
  virtual void receiveMessage(cyclus::msg_ptr msg);

  /**
     Transacted resources are extracted through this method       
     @param order the msg/order for which resource(s) are to be prepared 
     @return list of resources to be sent for this order       
   */ 
  virtual std::vector<cyclus::rsrc_ptr> removeResource(cyclus::Transaction order);

  /**
     Transacted resources are received through this method 
     @param trans the transaction to which these resource objects belong 
     @param manifest is the set of resources being received 
   */ 
  virtual void addResource(cyclus::Transaction trans,
                           std::vector<cyclus::rsrc_ptr> manifest);
  /* --- */

  /* --- EnrichmentFacility Methods --- */  
  enrichment::Assays getAssays(cyclus::mat_rsrc_ptr mat);

  inline void set_in_commodity(std::string in_commod)
  {
    in_commodity_ = in_commod;
  }

  inline std::string in_commodity() 
  {
    return in_commodity_;
  }   
    
  inline void set_out_commodity(std::string out_commod)
  {
    out_commodity_ = out_commod;
  }

  inline std::string out_commodity() 
  {
    return out_commodity_;
  }   
    
  inline void set_in_recipe(std::string in_recipe)
  {
    in_recipe_ = in_recipe;
  }   

  inline std::string in_recipe() 
  {
    return in_recipe_;
  }   
  
  inline void setMaxInventorySize(double size)
  {
    inventory_.setCapacity(size);
  }
    
  inline double maxInventorySize() 
  {
    return inventory_.capacity();
  }
    
  inline double inventoryQty() 
  {
    return inventory_.quantity();
  }
    
  inline void set_feed_assay(double assay)
  {
    feed_assay_ = assay;
  }   

  inline double feed_assay()
  {
    return feed_assay_;
  }

  inline void set_tails_assay(double assay)
  {
    tails_assay_ = assay;
  }   

  inline double tails_assay()
  {
    return tails_assay_;
  }

  inline void set_commodity_price(double price)
  {
    commodity_price_ = price;
  }   

  inline double commodity_price()
  {
    return commodity_price_;
  }
  /* --- */

 protected:  
  /* --- cyclus::Transaction Methods --- */     
  /**
     sends a transaction as an offer 
   */
  void sendOffer(cyclus::Transaction trans);

  /**
   */
  void makeRequest();

  /**
   */
  void makeOffer();

  /**
   */
  cyclus::Transaction buildTransaction();
  /* --- */

  /* --- EnrichmentFacility Members and Methods --- */  
  /**
   */
  void processOutgoingMaterial();

  /**
   */
  void recordEnrichment(double natural_u, double swu);

  std::string in_commodity_;

  std::string out_commodity_;

  std::string in_recipe_;

  double commodity_price_;

  double feed_assay_;
  
  double tails_assay_;  

  MatBuff inventory_;

  static int entry_;
  
  ///   A list of orders to be processed on the Tock 
  std::deque<cyclus::msg_ptr> orders_;
  /* --- */
};
#endif

