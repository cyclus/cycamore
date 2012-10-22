// EnrichmentFacility.h
#ifndef _ENRICHMENTFACILITY_H
#define _ENRICHMENTFACILITY_H

#include "FacilityModel.h"

#include "MatBuff.h"
#include "Table.h"
#include "Transaction.h"

#include <string>
#include <deque>

namespace Enrichment
{
  /**
     a simple container class for enrichment assays
   */
  class Assays
  {
  public:
    /// constructor
    Assays(double feed, double product, double tails);
    
    /// returns the feed assay
    double feed();

    /// returns the product assay
    double product();

    /// returns the tails assay
    double tails();

  private:
    double feed_, product_, tails_;
  };

  double uranium_assay(mat_rsrc_ptr mat);
  double uranium_qty(mat_rsrc_ptr mat);
  double feed_qty(double product_qty, Assays& assays);
  double tails_qty(double product_qty, Assays& assays);
  double swu_required(double product_qty, Assays& assays);
  double value_func(double frac);
};

// forward declarations
class QueryEngine;

/**
   @class EnrichmentFacility 
    
   @section introduction Introduction 
 */
class EnrichmentFacility : public FacilityModel
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
     @param qe a pointer to a QueryEngine object containing initialization data
   */
  virtual void initModuleMembers(QueryEngine* qe);

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
  virtual void cloneModuleMembersFrom(FacilityModel* sourceModel);
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

  /* --- Transaction Methods --- */
  /**
     When this facility receives a message, execute the transaction 
   */
  virtual void receiveMessage(msg_ptr msg);

  /**
     Transacted resources are extracted through this method       
     @param order the msg/order for which resource(s) are to be prepared 
     @return list of resources to be sent for this order 
      
   */ 
  virtual std::vector<rsrc_ptr> removeResource(Transaction order);

  /**
     Transacted resources are received through this method 
     @param trans the transaction to which these resource objects belong 
     @param manifest is the set of resources being received 
   */ 
  virtual void addResource(Transaction trans,
                           std::vector<rsrc_ptr> manifest);
  /* --- */

  /* --- EnrichmentFacility Methods --- */  
  Enrichment::Assays getAssays(mat_rsrc_ptr mat);

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
  /* --- Transaction Methods --- */     
  /**
     sends a transaction as an offer 
   */
  void sendOffer(Transaction trans);

  /**
   */
  void makeRequest();

  /**
   */
  void makeOffer();

  /**
   */
  Transaction buildTransaction();
  /* --- */

  /* --- EnrichmentFacility Members and Methods --- */  
  /**
   */
  void processOutgoingMaterial();

  /**
   */
  void define_table();

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

  static table_ptr table_;
  
  ///   A list of orders to be processed on the Tock 
  std::deque<msg_ptr> orders_;
  /* --- */
};
#endif

