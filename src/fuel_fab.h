#ifndef CYCAMORE_SRC_FUEL_FAB_H_
#define CYCAMORE_SRC_FUEL_FAB_H_

#include <string>
#include "cyclus.h"
#include "kitlus/sell_policy.h"
#include "kitlus/buy_policy.h"

using kitlus::BuyPolicy;
using kitlus::SellPolicy;

namespace cycamore {

class FuelFab : public cyclus::Facility {
 public:
  FuelFab(cyclus::Context* ctx);
  virtual ~FuelFab() {};
  virtual std::string str() {return "";};

  virtual void EnterNotify();

  #pragma cyclus

  virtual void Tick();
  virtual void Tock() {};

 private:
  #pragma cyclus var {}
  std::vector<std::string> fill_commods;
  #pragma cyclus var {}
  std::vector<std::string> fill_commod_prefs;
  #pragma cyclus var {}
  std::string fill_recipe;
  #pragma cyclus var {}
  double fill_size;
  #pragma cyclus var {'capacity': 'fill_size'}
  cyclus::toolkit::ResBuf<Material> fill;

  #pragma cyclus var {}
  std::vector<std::string> fiss_commods;
  #pragma cyclus var {}
  std::vector<std::string> fiss_commod_prefs;
  #pragma cyclus var {}
  double fiss_size;
  #pragma cyclus var {'capacity': 'fiss_size'}
  cyclus::toolkit::ResBuf<Material> fiss;

  #pragma cyclus var {}
  std::string topup_commod;
  #pragma cyclus var {}
  double topup_size;
  #pragma cyclus var {'capacity': 'topup_size'}
  cyclus::toolkit::ResBuf<Material> topup;

  #pragma cyclus var {}
  std::string outcommod;

  #pragma cyclus var { \
    "doc": "'fission' for fast reactor compositions or 'thermal' for slow reactors.", \
  }
  std::string spectrum;

  #pragma cyclus var {}
  double throughput;

  BuyPolicy fillpolicy_;
  BuyPolicy fisspolicy_;
};

class FissConverter : public cyclus::Converter<cyclus::Material> {
 public:
  FissConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum,
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~FissConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    int check = CosiCheck(w_tgt, w_fill_, w_fiss_);
    int check_topup = CosiCheck(w_tgt, w_fill_, w_topup_);

    if (check == 0) {
      return CosiFissileFrac(w_tgt, w_fill_, w_fiss_) * m->quantity();
    } else if (check_topup == 0) {
      // use fiss inventory as filler, and topup as fissile
      return 1 - CosiFissileFrac(w_tgt, w_fiss_, w_topup_) * m->quantity();
    } else {
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

class FillConverter : public cyclus::Converter<cyclus::Material> {
 public:
  FillConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum,
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~FillConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    int check = CosiCheck(w_tgt, w_fill_, w_fiss_);
    int check_topup = CosiCheck(w_tgt, w_fill_, w_topup_);

    if (check == 0) {
      return 1 - CosiFissileFrac(w_tgt, w_fill_, w_fiss_) * m->quantity();
    } else if (check_topup == 0) {
      // switched fissile inventory to filler so don't need any filler inventory
      return 0;
    } else {
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

class TopupConverter : public cyclus::Converter<cyclus::Material> {
 public:
  TopupConverter(
      double w_fill,
      double w_fiss,
      double w_topup,
      std::string spectrum,
      ) : w_fiss_(w_fiss), w_topup_(w_topup), w_fill_(w_fill), spec_(spectrum) {}

  virtual ~TopupConverter() {}

  virtual double convert(
      cyclus::Material::Ptr m,
      cyclus::Arc const * a = NULL,
      cyclus::ExchangeTranslationContext<cyclus::Material> const * ctx = NULL) const {

    double w_tgt = CosiWeight(m->comp(), spec_);
    int check = CosiCheck(w_tgt, w_fill_, w_fiss_);
    int check_topup = CosiCheck(w_tgt, w_fill_, w_topup_);

    if (check == 0) {
      return 0;
    } else if (check_topup == 0) {
      // switched fissile inventory to filler and topup as fissile
      return CosiFissileFrac(w_tgt, w_fiss_, w_topup_) * m->quantity();
    } else {
      return 1e200;
    }
  }

 private:
  std::string spec_;
  double w_fiss_;
  double w_topup_;
  double w_fill_;
};

} // namespace cycamore


#endif  // CYCAMORE_SRC_FUEL_FAB_H_
