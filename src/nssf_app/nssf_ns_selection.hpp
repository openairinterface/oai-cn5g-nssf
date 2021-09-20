/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
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

/*! \file nssf_ns_selection.hpp
  \brief
  \author  Rohan Kharade
  \company Openairinterface.org
  \email:  rohan.kharade@openairinterface.org
*/

#ifndef FILE_NSSF_NS_SELEION_HPP_SEEN
#define FILE_NSSF_NS_SELEION_HPP_SEEN

#include "3gpp_29.531.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <shared_mutex>
#include <string>

namespace nssf {

class ns_select_registration
    : public std::enable_shared_from_this<ns_select_registration> {
public:
  ns_select_registration();
  ns_select_registration(ns_select_registration const &) = delete;
  void operator=(ns_select_registration const &) = delete;

protected:
  std::vector<subscribed_snssai_t> *subscribed_nssai;
  allowed_nssai_t *allowed_nssai_current_access;
  allowed_nssai_t *allowed_nssai_other_access;
  std::vector<snssai_t> *snssai_for_mapping;
  std::vector<mapping_of_snssai_t> *mapping_of_nssai;
  std::vector<snssai_t> *requested_nssai;
  bool request_mapping;
  bool default_configured_snssai_ind;
};

class ns_select_pdu_session
    : public std::enable_shared_from_this<ns_select_pdu_session> {
public:
  ns_select_pdu_session();
  ns_select_pdu_session(ns_select_pdu_session const &) = delete;
  void operator=(ns_select_pdu_session const &) = delete;

protected:
  snssai_t snssai;
  std::string roaming_indication;
  snssai_t *home_snssai;
};

class ns_select_ue_config_update
    : public std::enable_shared_from_this<ns_select_ue_config_update> {
public:
  ns_select_ue_config_update();
  ns_select_ue_config_update(ns_select_ue_config_update const &) = delete;
  void operator=(ns_select_ue_config_update const &) = delete;

protected:
  std::vector<subscribed_snssai_t> *subscribed_snssai;
  allowed_nssai_t *allowed_nssai_current_access;
  allowed_nssai_t *allowed_nssai_other_access;
  bool default_configured_snssai_ind;
  std::vector<snssai_t> *requested_nssai;
  std::vector<mapping_of_snssai_t> *mapping_of_nssai;
};

} // namespace nssf

#endif