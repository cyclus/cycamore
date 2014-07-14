#include "separationmatrix_facility.h"

namespace separationmatrix {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationmatrixFacility::SeparationmatrixFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationmatrixFacility::str() {
  return Facility::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Tick() {

  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is ticking at time "
                                   << time << " {";

  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tick are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();

  // if lifetime is up, clear self of materials??

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tick are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Tock() {
  int time = context()->time();
  LOG(cyclus::LEV_INFO3, "SepMtx") << prototype() << " is tocking {";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the beginning of the tock are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();

  BeginProcessing_();
  Separate_();

  LOG(cyclus::LEV_DEBUG3, "SepMtx") << "Current facility parameters for "
                                    << prototype()
                                    << " at the end of the tock are:";
  LOG(cyclus::LEV_DEBUG4, "SepMtx") << "    ProcessTime: " << process_time_();
  LOG(cyclus::LEV_INFO3, "SepMtx") << "}";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationmatrixFacility::inventory_quantity(std::string commod)const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = inventory.find(commod);
  double amt;
  if ( found != inventory.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double SeparationmatrixFacility::inventory_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for( it = inventory.begin(); it != inventory.end(); ++it) {
    total += inventory_quantity((*it).first);
  }
  return total;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Separate_(){
  Separate_(processing[ready()]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Separate_(cyclus::toolkit::ResourceBuff buff){
  using cyclus::Material;
  using cyclus::ResCast;

  while ( !buff.empty() ){
    Material::Ptr back = ResCast<Material>(buff.back());
    buff.pop_back();
    Separate_(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Separate_(cyclus::Material::Ptr mat){

  std::map<std::string, cyclus::Composition> sep_streams;
  cyclus::CompMap::iterator entry;
  for (entry = mat.comp().begin(); entry != mat.comp().end(); ++it){
    Element elem = Element(entry.first) 
    double sep = entry.second*eff(elem);
    cyclus::Commodity commod = out_commod(elem);
    sep_stream[commod][iso] = sep;
  }
  inventory[commod].Push(mat.Extract(sep_stream[iso]));
  inventory[waste_commod].Push(mat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr SeparationsmatrixFacility::CollapseBuff(cyclus::toolkit::ResourceBuff to_collapse){
  using cyclus::toolkit::Manifest;
  using cyclus::Material;
  using cyclus::ResCast;
  double qty =  to_collapse.quantity();
  Manifest manifest = to_collapse.PopQty(qty);

  Material::Ptr back = ResCast<Material>(manifest.back());
  manifest.pop_back();
  while ( !manifest.empty() ){
    back->Absorb(ResCast<Material>(manifest.back()));
    manifest.pop_back();
  }
  return back;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::BeginProcessing_(){
  LOG(cyclus::LEV_DEBUG2, "SBlend") << "CommodConverter " << prototype()
                                    << " added resources to processing";
  std::map<std::string, cyclus::toolkit::ResourceBuff>::iterator it;
  for (it = inventory.begin(); it != inventory.end(); ++it){
    while (!(*it).second.empty()){
      try {
        processing[context()->time()][(*it).first].Push((*it).second.Pop());
      } catch(cyclus::Error& e) {
        e.msg(Agent::InformErrorMsg(e.msg()));
        throw e;
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparationmatrixFacility(cyclus::Context* ctx) {
  return new SeparationmatrixFacility(ctx);
}

}
