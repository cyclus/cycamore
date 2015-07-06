// storage.cc
// Implements the Storage class
#include "storage.h"

namespace storage {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Storage::Storage(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the Storage is experimental.");
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
  Commodity commod = Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, throughput);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::EnterNotify() {
  Facility::EnterNotify();
  buy_policy.Init(this, &inventory, std::string("inventory"), throughput).Set(in_commod).Start();
  sell_policy.Init(this, &stocks, std::string("stocks"), throughput).Set(out_commod).Start();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Storage::str() {
  std::stringstream ss;
  std::string ans;
  if (cyclus::toolkit::CommodityProducer::
      Produces(cyclus::toolkit::Commodity(out_commod_()))){
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodity = " << in_commod_() << ",\n"
     << "     Output Commodity = " << out_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Throughput = " << throughput_() << ",\n"
     << " commod producer members: " << " produces "
     << out_commod << "?:" << ans
     << " throughput: " << cyclus::toolkit::CommodityProducer::Capacity(out_commod_())
     << " cost: " << cyclus::toolkit::CommodityProducer::Cost(out_commod_())
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tick() { 
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is ticking {";

  if (current_capacity() > cyclus::eps()) {
    LOG(cyclus::LEV_INFO4, "ComCnv") << " has capacity for " << current_capacity()
                                       << " kg of " << in_commod
                                       << " recipe: " << in_recipe << ".";
  }

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::Tock() {
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is tocking {";

  BeginProcessing_(); // place unprocessed inventory into processing

  if( ready() >= 0 || process_time_() == 0 ) {
    ProcessMat_(throughput_()); // place processing into stocks
  }

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::AddMat_(cyclus::Material::Ptr mat) {
  // Here we do not check that the recipe matches the input recipe. 

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " is initially holding "
                                << inventory.quantity() << " total.";

  try {
    inventory.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " added " << mat->quantity()
                                << " of " << in_commod_()
                                << " to its inventory, which is holding "
                                << inventory.quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::BeginProcessing_(){
  while( inventory.count() > 0 ){
    try {
      processing[context()->time()].Push(inventory.Pop());
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

  int t = ready();

  if ( ProcessingAmt_(t) > 0 ){
    try {
      double to_pop = std::min(cap, processing[t].quantity());

      // pop appropriate amount of material from processing 
      // Push process ready material to stocks
      stocks.Push(processing[t].Pop(to_pop));

      AdvanceUnconverted_(t);
      LOG(cyclus::LEV_INFO1, "ComCnv") << "Storage " << prototype() 
                                        << " converted quantity : " << to_pop 
                                        << " from " << in_commod 
                                        << " to " << out_commod
                                        << " at t= " << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double Storage::ProcessingAmt_(int time) {
  using cyclus::toolkit::ResBuf;
  double to_ret = 0;
  std::map<int, ResBuf<cyclus::Material> >::iterator proc = processing.find(time);
  if ( proc!=processing.end() && 
      proc->second.quantity() > 0 ) {
    to_ret = proc->second.quantity();
  }
  return to_ret;

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Storage::AdvanceUnconverted_(int time){
  using cyclus::Material;
  using cyclus::ResCast;

  double to_pop = ProcessingAmt_(time);
  if ( to_pop > 0 ) {
    try {
      processing[time+1].Push(processing[time].Pop(to_pop));
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStorage(cyclus::Context* ctx) {
  return new Storage(ctx);
}

} // namespace storage