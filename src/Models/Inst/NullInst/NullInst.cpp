// NullInst.cpp
// Implements the NullInst class

#include "NullInst.h"

using namespace std;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullInst::NullInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NullInst::~NullInst() {}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" Model* constructNullInst() {
  return new NullInst();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" void destructNullInst(Model* model) {
  delete model;
}

/* ------------------- */ 



