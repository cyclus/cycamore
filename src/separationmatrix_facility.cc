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
  for(it=inventory.begin(); it != inventory.end(); ++it) {
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
  using cyclus::toolkit::ResourceBuff;

  while ( !buff.empty() ){
    Material::Ptr back = ResCast<Material>(buff.Pop(ResourceBuff::BACK));
    Separate_(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationmatrixFacility::Separate_(cyclus::Material::Ptr mat){
  using cyclus::CompMap;
  using cyclus::Composition;
  using std::make_pair;
  using std::string;

  std::map<string, CompMap> sep_comps;
  CompMap::iterator entry;
  CompMap orig = mat->comp()->mass();
  for (entry = orig.begin(); entry != orig.end(); ++entry){
    int iso = int(entry->first);
    int elem = int(iso/10000000.); // convert iso to element
    double sep = entry->second*Eff_(elem); // access matrix
    string stream = Stream_(elem);
    sep_comps[stream][iso] = sep;
  }
  std::map< string, CompMap >::iterator str;
  for(str=sep_comps.begin(); str!=sep_comps.end(); ++str){
    CompMap to_extract = (*str).second;
    double qty = cyclus::compmath::Sum(to_extract);
    Composition::Ptr c = Composition::CreateFromMass(to_extract);
    inventory[(*str).first].Push(mat->ExtractComp(qty, c));
  }
  inventory[waste_stream].Push(mat);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int SeparationmatrixFacility::ElemIdx_(int element){
  int to_ret = find(elems.begin(), elems.end(), element) - elems.begin();

  if( to_ret > elems.size() ){
    throw cyclus::KeyError("The element was not found in the matrix");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SeparationmatrixFacility::Eff_(int element){
  double to_ret;
  try {
    int idx = ElemIdx_(element);
    to_ret = boost::lexical_cast<double>(effs[idx]);
  } catch (cyclus::KeyError &e) {
    to_ret = 0;
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationmatrixFacility::Stream_(int element){
  double to_ret;
  try {
    int idx = ElemIdx_(element);
    to_ret = streams[idx];
  } catch (cyclus::KeyError &e) {
    to_ret = waste_stream_();
  }
  return to_ret;
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
