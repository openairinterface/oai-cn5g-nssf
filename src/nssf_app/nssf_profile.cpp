/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 *file except in compliance with the License. You may obtain a copy of the
 *License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file nssf_nf_profile.cpp
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: Tien-Thinh.Nguyen@eurecom.fr
 */

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "logger.hpp"
#include "nssf_profile.hpp"
#include "string.hpp"

using namespace nssf;

//------------------------------------------------------------------------------
void nssf_profile::set_nf_instance_id(const std::string &instance_id) {
  nf_instance_id = instance_id;
}

//------------------------------------------------------------------------------
void nssf_profile::get_nf_instance_id(std::string &instance_id) const {
  instance_id = nf_instance_id;
}

//------------------------------------------------------------------------------
std::string nssf_profile::get_nf_instance_id() const { return nf_instance_id; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_instance_name(const std::string &instance_name) {
  nf_instance_name = instance_name;
}

//------------------------------------------------------------------------------
void nssf_profile::get_nf_instance_name(std::string &instance_name) const {
  instance_name = nf_instance_name;
}

//------------------------------------------------------------------------------
std::string nssf_profile::get_nf_instance_name() const {
  return nf_instance_name;
}

//------------------------------------------------------------------------------
void nssf_profile::set_nf_type(const std::string &type) { nf_type = type; }

//------------------------------------------------------------------------------
std::string nssf_profile::get_nf_type() const { return nf_type; }
//------------------------------------------------------------------------------
void nssf_profile::set_nf_status(const std::string &status) {
  nf_status = status;
}

//------------------------------------------------------------------------------
void nssf_profile::get_nf_status(std::string &status) const {
  status = nf_status;
}

//------------------------------------------------------------------------------
std::string nssf_profile::get_nf_status() const { return nf_status; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_heartBeat_timer(const int32_t &timer) {
  heartBeat_timer = timer;
}

//------------------------------------------------------------------------------
void nssf_profile::get_nf_heartBeat_timer(int32_t &timer) const {
  timer = heartBeat_timer;
}

//------------------------------------------------------------------------------
int32_t nssf_profile::get_nf_heartBeat_timer() const { return heartBeat_timer; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_priority(const uint16_t &p) { priority = p; }

//------------------------------------------------------------------------------
void nssf_profile::get_nf_priority(uint16_t &p) const { p = priority; }

//------------------------------------------------------------------------------
uint16_t nssf_profile::get_nf_priority() const { return priority; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_capacity(const uint16_t &c) { capacity = c; }

//------------------------------------------------------------------------------
void nssf_profile::get_nf_capacity(uint16_t &c) const { c = capacity; }

//------------------------------------------------------------------------------
uint16_t nssf_profile::get_nf_capacity() const { return capacity; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_snssais(const std::vector<snssai_t> &s) {
  snssais = s;
}

//------------------------------------------------------------------------------
void nssf_profile::get_nf_snssais(std::vector<snssai_t> &s) const {
  s = snssais;
}

//------------------------------------------------------------------------------
void nssf_profile::add_snssai(const snssai_t &s) { snssais.push_back(s); }

//------------------------------------------------------------------------------
void nssf_profile::set_fqdn(const std::string &fqdN) { fqdn = fqdN; }

//------------------------------------------------------------------------------
std::string nssf_profile::get_fqdn() const { return fqdn; }

//------------------------------------------------------------------------------
void nssf_profile::set_nf_ipv4_addresses(const std::vector<struct in_addr> &a) {
  ipv4_addresses = a;
}

//------------------------------------------------------------------------------
void nssf_profile::add_nf_ipv4_addresses(const struct in_addr &a) {
  ipv4_addresses.push_back(a);
}
//------------------------------------------------------------------------------
void nssf_profile::get_nf_ipv4_addresses(std::vector<struct in_addr> &a) const {
  a = ipv4_addresses;
}

//------------------------------------------------------------------------------
void nssf_profile::display() const {
  Logger::nssf_app().debug("- NF instance info");
  Logger::nssf_app().debug("    Instance ID: %s", nf_instance_id.c_str());
  Logger::nssf_app().debug("    Instance name: %s", nf_instance_name.c_str());
  Logger::nssf_app().debug("    Instance type: %s", nf_type.c_str());
  Logger::nssf_app().debug("    Instance fqdn: %s", fqdn.c_str());
  Logger::nssf_app().debug("    Status: %s", nf_status.c_str());
  Logger::nssf_app().debug("    HeartBeat timer: %d", heartBeat_timer);
  Logger::nssf_app().debug("    Priority: %d", priority);
  Logger::nssf_app().debug("    Capacity: %d", capacity);
  // SNSSAIs
  if (snssais.size() > 0) {
    Logger::nssf_app().debug("    SNSSAI:");
  }
  for (auto s : snssais) {
    Logger::nssf_app().debug("        SST, SD: %d, %s", s.sST, s.sD.c_str());
  }

  // IPv4 Addresses
  if (ipv4_addresses.size() > 0) {
    Logger::nssf_app().debug("    IPv4 Addr:");
  }
  for (auto address : ipv4_addresses) {
    Logger::nssf_app().debug("        %s", inet_ntoa(address));
  }
}

//------------------------------------------------------------------------------
void nssf_profile::to_json(nlohmann::json &data) const {
  data["nfInstanceId"] = nf_instance_id;
  data["nfInstanceName"] = nf_instance_name;
  data["nfType"] = nf_type;
  data["nfStatus"] = nf_status;
  data["heartBeatTimer"] = heartBeat_timer;
  // SNSSAIs
  data["sNssais"] = nlohmann::json::array();
  for (auto s : snssais) {
    nlohmann::json tmp = {};
    tmp["sst"] = s.sST;
    tmp["sd"] = s.sD;
    data["sNssais"].push_back(tmp);
  }
  data["fqdn"] = fqdn;
  // ipv4_addresses
  data["ipv4Addresses"] = nlohmann::json::array();
  for (auto address : ipv4_addresses) {
    nlohmann::json tmp = inet_ntoa(address);
    data["ipv4Addresses"].push_back(tmp);
  }

  data["priority"] = priority;
  data["capacity"] = capacity;

  Logger::nssf_app().debug("NSSF profile to JSON:\n %s", data.dump().c_str());
}

//------------------------------------------------------------------------------
void nssf_profile::from_json(const nlohmann::json &data) {
  if (data.find("nfInstanceId") != data.end()) {
    nf_instance_id = data["nfInstanceId"].get<std::string>();
  }

  if (data.find("nfInstanceName") != data.end()) {
    nf_instance_name = data["nfInstanceName"].get<std::string>();
  }

  if (data.find("nfType") != data.end()) {
    nf_type = data["nfType"].get<std::string>();
  }

  if (data.find("nfStatus") != data.end()) {
    nf_status = data["nfStatus"].get<std::string>();
  }

  if (data.find("heartBeatTimer") != data.end()) {
    heartBeat_timer = data["heartBeatTimer"].get<int>();
  }
  // sNssais
  if (data.find("sNssais") != data.end()) {
    for (auto it : data["sNssais"]) {
      snssai_t s = {};
      s.sST = it["sst"].get<int>();
      s.sD = it["sd"].get<std::string>();
      snssais.push_back(s);
    }
  }

  if (data.find("ipv4Addresses") != data.end()) {
    nlohmann::json addresses = data["ipv4Addresses"];

    for (auto it : addresses) {
      struct in_addr addr4 = {};
      std::string address = it.get<std::string>();
      unsigned char buf_in_addr[sizeof(struct in_addr)];
      if (inet_pton(AF_INET, util::trim(address).c_str(), buf_in_addr) == 1) {
        memcpy(&addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::nssf_app().warn("Address conversion: Bad value %s",
                                util::trim(address).c_str());
      }
      add_nf_ipv4_addresses(addr4);
    }
  }

  if (data.find("priority") != data.end()) {
    priority = data["priority"].get<int>();
  }

  if (data.find("capacity") != data.end()) {
    capacity = data["capacity"].get<int>();
  }
  display();
}

//------------------------------------------------------------------------------
void nssf_profile::handle_heartbeart_timeout(uint64_t ms) {
  Logger::nssf_app().info("Handle heartbeart timeout profile %s, time %d",
                          nf_instance_id.c_str(), ms);
  set_nf_status("SUSPENDED");
}
