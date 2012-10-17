// EnrichmentFacility.cpp
// Implements the EnrichmentFacility class
#include "EnrichmentFacility.h"

#include "QueryEngine.h"
#include "Logger.h"
#include "CycException.h"
#include <sstream>
#include <limits>

#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::EnrichmentFacility()
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
EnrichmentFacility::~EnrichmentFacility() 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::initModuleMembers(QueryEngine* qe) 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::string EnrichmentFacility::str() 
{
  std::stringstream ss;
  ss << FacilityModel::str();
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::cloneModuleMembersFrom(FacilityModel* sourceModel) 
{
  EnrichmentFacility* source = dynamic_cast<EnrichmentFacility*>(sourceModel);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTick(int time)
{
  LOG(LEV_INFO3, "EnrFac") << facName() << " is tocking {";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::handleTock(int time)
{
  LOG(LEV_INFO3, "EnrFac") << facName() << " is tocking {";
  LOG(LEV_INFO3, "EnrFac") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::receiveMessage(msg_ptr msg)
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::addResource(Transaction trans,
                                     std::vector<rsrc_ptr> manifest)
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
std::vector<rsrc_ptr> EnrichmentFacility::removeResource(Transaction order) 
{
  return vector<rsrc_ptr>();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void EnrichmentFacility::sendOffer(Transaction trans) 
{
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
extern "C" Model* constructEnrichmentFacility() 
{
  return new EnrichmentFacility();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructEnrichmentFacility(Model* model) 
{
      delete model;
}
