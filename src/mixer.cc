#include <sstream>

#include "mixer.h"

namespace cycamore {

Mixer::Mixer(cyclus::Context* ctx) : cyclus::Facility(ctx), throughput(0) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>(
      "the Mixer archetype is experimental");
}

cyclus::Inventories Mixer::SnapshotInv() {
  cyclus::Inventories invs;

  // these inventory names are intentionally convoluted so as to not clash
  // with the user-specified stream commods that are used as the Mixer
  // streams inventory names.
  invs["output-inv-name"] = output.PopNRes(output.count());
  output.Push(invs["output-inv-name"]);

  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> >::iterator
      it;
  for (it = streambufs.begin(); it != streambufs.end(); ++it) {
    invs[it->first] = it->second.PopNRes(it->second.count());
    it->second.Push(invs[it->first]);
  }
  return invs;
}

void Mixer::InitInv(cyclus::Inventories& inv) {
  inv["output-inv-name"] = output.PopNRes(output.count());
  output.Push(inv["output-inv-name"]);

  cyclus::Inventories::iterator it;
  for (it = inv.begin(); it != inv.end(); ++it) {
    streambufs[it->first].Push(it->second);
  }
}

void Mixer::EnterNotify() {
  cyclus::Facility::EnterNotify();

  for (int i = 0; i < streams_.size(); i++) {
    mixing_ratios.push_back(streams_[i]->first->first);
    in_buf_sizes.push_back(streams_[i]->first->second);
    
    std::vector<pair<string, pref> stream_commods_;
    for (int j = 0; j <  streams_[i]->second.size(); j++) {
        stream_commods_.push_back(streams_[i]->second[j]);
    }
    in_commods.push_back(stream_commods_);
  }



  if (in_commod_prefs.empty()) {
    for (int i = 0; i < in_commods.size(); i++) {
      in_commod_prefs.push_back(1);
    }
  } else if (in_commod_prefs.size() != in_commods.size()) {
    std::stringstream ss;
    ss << "prototype '" << prototype() << "' has " << mixing_ratios.size()
       << " commodity preferences values, expected " << in_commods.size();
    throw cyclus::ValidationError(ss.str());
  }

  if (mixing_ratios.empty()) {
    for (int i = 0; i < in_commods.size(); i++) {
      mixing_ratios.push_back(1.0 / in_commods.size());
    }
  } else if (mixing_ratios.size() != in_commods.size()) {
    std::stringstream ss;
    ss << "prototype '" << prototype() << "' has " << mixing_ratios.size()
       << " commodity fraction values, expected " << in_commods.size();
    throw cyclus::ValidationError(ss.str());

  } else {
    double frac_sum = 0.0;
    for (int i = 0; i < mixing_ratios.size(); i++) frac_sum += mixing_ratios[i];

    if (frac_sum != 1.0) {
      std::stringstream ss;
      ss << "prototype '" << prototype() << "': the sum of mixing fractions is "
                                            "not 1, renormalization will be "
                                            "done.";
      cyclus::Warn<cyclus::VALUE_WARNING>(ss.str());
    }

    for (int i = 0; i < mixing_ratios.size(); i++) {
      mixing_ratios[i] *= 1.0 / frac_sum;
    }
  }

  for (int i = 0; i < in_commods.size(); i++) {
    std::string name = "in_stream_" + std::to_string(i);
    double cap = in_buf_sizes[i];
    if (cap >= 0) {
      streambufs[name].capacity(cap);
    }
  }

  sell_policy.Init(this, &output, "output").Set(out_commod).Start();
}

void Mixer::Tick() {
  if (output.quantity() < output.capacity()) {
    double tgt_qty = output.space();

    for (int i = 0; i < in_commods.size(); i++) {
      std::string name = "in_stream_" + std::to_string(i);
      tgt_qty =
          std::min(tgt_qty, streambufs[name].quantity() / mixing_ratios[i]);
    }

    tgt_qty = std::min(tgt_qty, throughput);

    if (tgt_qty > 0) {
      cyclus::Material::Ptr m;
      for (int i = 0; i < in_commods.size(); i++) {
        std::string name = in_commods[i];
        double pop_qty = mixing_ratios[i] * tgt_qty;
        if (i == 0) {
          m = streambufs[name].Pop(pop_qty, cyclus::eps_rsrc());
        } else {
          cyclus::Material::Ptr m_ =
              streambufs[name].Pop(pop_qty, cyclus::eps_rsrc());
          m->Absorb(m_);
        }
      }
      output.Push(m);
    }
  }
}

std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
Mixer::GetMatlRequests() {
  using cyclus::RequestPortfolio;

  std::set<RequestPortfolio<cyclus::Material>::Ptr> ports;
  
  for (int i = 0; i < in_commods.size(); i++) {
    std::string name = "in_stream_" + std::to_string(i);
    
    if (streambufs[name].space() > cyclus::eps_rsrc()) {
      RequestPortfolio<cyclus::Material>::Ptr port(
          new RequestPortfolio<cyclus::Material>());

      cyclus::Material::Ptr m;
      m = cyclus::NewBlankMaterial(streambufs[name].space());

      std::vector<cyclus::Request<Material>*> reqs;      
      for (int j = 0; j < in_commods[i].size(); j++) {
        std::string commod = in_commods[i][j]->first;
        double prefs = in_commods[i][j]->second;
        reqs = port->AddRequest(m, this, commod , pref, false);
        req_inventories_[reqs.back()] = name;
      }
      port->AddMutualReqs(reqs);  
      ports.insert(port);
    }
  }
  return ports;
}

void Mixer::AcceptMatlTrades(
    const std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                                cyclus::Material::Ptr> >& responses) {
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> >::const_iterator trade;

  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    cyclus::Request<cyclus::Material>* req = trade->first.request;
    cyclus::Material::Ptr m = trade->second;

    std::string name = req_inventories_[req];
    bool assigned = false;
    std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> >::iterator
        it;

    for (it = streambufs.begin(); it != streambufs.end(); it++) {
      if (name == it->first) {
        it->second.Push(m);
        assigned = true;
        break;
      }
    }
    if (!assigned) {
      throw cyclus::ValueError("cycamore::Mixer was overmatched on requests");
    }
  }

  req_inventories_.clear();
}

extern "C" cyclus::Agent* ConstructMixer(cyclus::Context* ctx) {
  return new Mixer(ctx);
}
}
