// stub_converter.cc
// Implements the StubConverter class

#include "logger.h"
#include "error.h"
#include "input_xml.h"
#include "stub_converter.h"

/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubConverter::init(xmlNodePtr cur) {
  cyclus::ConverterModel::init(cur);
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubConverter::copy(StubConverter* src) {
  cyclus::ConverterModel::copy(src);
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StubConverter::copyFreshModel(cyclus::Model* src) {
  copy(dynamic_cast<StubConverter*>(src));
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StubConverter::str() {
  return cyclus::ConverterModel::str();
};

/* ------------------- */


/* --------------------
 * all CONVERTERMODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Message::Ptr StubConverter::convert(cyclus::Message::Ptr convMsg, cyclus::Message::Ptr refMsg) {
  throw cyclus::CycException("The StubConverter should not be used to convert things.");
}

/* ------------------- */


/* --------------------
 * all MODEL classes have these members
 * --------------------
 */

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Model* constructStubConverter() {
    return new StubConverter();
}

/* ------------------- */


