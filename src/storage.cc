// storage.cc
// Implements the Storage class
#include "storage.h"

namespace storage {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Storage::Storage(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("The Storage Facility is experimental.");
    };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema storage::Storage

#pragma cyclus def annotations storage::Storage

#pragma cyclus def initinv storage::Storage

#pragma cyclus def snapshotinv storage::Storage

#pragma cyclus def infiletodb storage::Storage

#pragma cyclus def snapshot storage::Storage

#pragma cyclus def clone storage::Storage

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::InitFrom(Storage* m) {

  #pragma cyclus impl initfromcopy storage::Storage
  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::InitFrom(cyclus::QueryableBackend* b){

  #pragma cyclus impl initfromdb storage::Storage

  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commods[0]);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, throughput);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::EnterNotify() {
  Facility::EnterNotify();
  buy_policy.Init(this, &inventory, std::string("inventory"));

  //dummy comp, use in_recipe if provided
  cyclus::Composition::Ptr comp;

  for(int i=0; i!=in_commods.size(); ++i) {
    buy_policy.Set(in_commods[i],comp,in_commod_prefs[i]);
  }
  buy_policy.Start();

  sell_policy.Init(this, &stocks, std::string("stocks")).Set(out_commods[0]).Start();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Storage::str() {
  std::stringstream ss;
  std::string ans;
  if (cyclus::toolkit::CommodityProducer::
      Produces(cyclus::toolkit::Commodity(out_commods[0]))){
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Output Commodity = " << out_commods[0] << ",\n"
     << "     Residence Time = " << residence_time_() << ",\n"
     << "     Throughput = " << throughput_() << ",\n"
     << " commod producer members: " << " produces "
     << out_commods[0] << "?:" << ans
     << " throughput: " << cyclus::toolkit::CommodityProducer::Capacity(out_commods[0])
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tick() { 
  // Set available capacity for Buy Policy
  inventory.capacity(current_capacity());

  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is ticking {";

  if (current_capacity() > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "ComCnv") << " has capacity for " << current_capacity()
                                       << " kg of material.";
  }
  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tock() {
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is tocking {";

  BeginProcessing_(); // place unprocessed inventory into processing

  if( ready_time() >= 0 || residence_time_() == 0  && !inventory.empty() ) {
    ReadyMatl_(ready_time());  // place processing into ready
  }

  ProcessMat_(throughput_()); // place ready into stocks

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::AddMat_(cyclus::Material::Ptr mat) {

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " is initially holding "
                                << inventory.quantity() << " total.";

  try {
    inventory.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " added " << mat->quantity()
                                << " of material to its inventory, which is holding "
                                << inventory.quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::BeginProcessing_(){
  while( inventory.count() > 0 ){
    try {
      processing.Push(inventory.Pop());
      entry_times.push_back(context()->time());

      LOG(cyclus::LEV_DEBUG2, "ComCnv") << "Storage " << prototype() 
                                      << " added resources to processing at t= "
                                      << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::ProcessMat_(double cap){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResBuf;
  using cyclus::toolkit::Manifest;

  if ( !ready.empty() ){
    try { 

      double max_pop = std::min(cap, ready.quantity());

      if ( batch_handling_() ) {
        if (max_pop == ready.quantity()) {
          stocks.Push(ready.PopN(ready.count()));
        }
        else {
          double cap_pop = ready.Peek()->quantity();
          while(cap_pop<=max_pop && !ready.empty() ){
            stocks.Push(ready.Pop());
            cap_pop += ready.empty() ? 0 : ready.Peek()->quantity();
          }
        }
      }
      else {
        stocks.Push(ready.Pop(max_pop));
      }

      LOG(cyclus::LEV_INFO1, "ComCnv") << "Storage " << prototype() 
                                        << " moved resources" 
                                        << " from ready to stocks" 
                                        << " at t= " << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::ReadyMatl_(int time) {
  using cyclus::toolkit::ResBuf;

  int to_ready = 0;

  while(!entry_times.empty() && entry_times.front()<=time){
    entry_times.pop_front();
    ++to_ready;
  }

  ready.Push(processing.PopN(to_ready));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStorage(cyclus::Context* ctx) {
  return new Storage(ctx);
}

} // namespace storage