#include "separationmatrix.h"

namespace separationmatrix {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SeparationMatrix::SeparationMatrix(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the SeparationMatrix is experimental.");
    };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string SeparationMatrix::str() {
  return Facility::str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Tick() {

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
void SeparationMatrix::Tock() {
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
const double SeparationMatrix::inventory_quantity(std::string commod)const {
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
const double SeparationMatrix::inventory_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for(it=inventory.begin(); it != inventory.end(); ++it) {
    total += inventory_quantity((*it).first);
  }
  return total;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(){
  Separate_(processing[ready()]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(cyclus::toolkit::ResourceBuff buff){
  using cyclus::Material;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  while ( !buff.empty() ){
    Material::Ptr back = ResCast<Material>(buff.Pop(ResourceBuff::BACK));
    Separate_(ResCast<Material>(back));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::Separate_(cyclus::Material::Ptr mat){
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
int SeparationMatrix::ElemIdx_(int element){
  int to_ret = find(elems.begin(), elems.end(), element) - elems.begin();

  if( to_ret > elems.size() ){
    throw cyclus::KeyError("The element was not found in the matrix");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SeparationMatrix::Eff_(int element){
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
std::string SeparationMatrix::Stream_(int element){
  std::string to_ret;
  try {
    int idx = ElemIdx_(element);
    to_ret = streams[idx];
  } catch (cyclus::KeyError &e) {
    to_ret = waste_stream_();
  }
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SeparationMatrix::BeginProcessing_(){
  using cyclus::toolkit::ResourceBuff;

  LOG(cyclus::LEV_DEBUG2, "SepMtx") << "Sepatations Matrix" << prototype()
                                    << " added resources to processing";
  while (!stocks.empty()){
    try {
      processing[context()->time()].Push(stocks.Pop(ResourceBuff::BACK));
    } catch(cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSeparationMatrix(cyclus::Context* ctx) {
  return new SeparationMatrix(ctx);
}

}
