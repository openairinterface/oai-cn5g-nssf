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

/*! \file nssf_sbi.hpp
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#ifndef FILEnssfnssf_HPP_SEEN
#define FILEnssfnssf_HPP_SEEN

#include <map>
#include <thread>

#include "3gpp_29.510.h"
#include "itti.hpp"
#include "nssf_profile.hpp"
#include <curl/curl.h>

namespace nssf {

#define TASK_NSSF_SBI_TIMEOUT_NRF_HEARTBEAT (1)
#define TASK_NSSF_SBI_TIMEOUT_NRF_DEREGISTRATION (2)

class nssf_nrf {
private:
  std::thread::id thread_id;
  std::thread thread;

  nssf_profile nssf_nf_profile; // NSSF profile
  std::string nssf_instance_id; // NSSF instance id
  timer_id_t timer_nssf_heartbeat;

public:
  nssf_nrf();
  nssf_nrf(nssf_nrf const &) = delete;
  void operator=(nssf_nrf const &) = delete;

  /*
   * Send NF instance registration to NRF
   * @param [const std::string &] url: nssf's URL
   * @return void
   */
  void send_register_nf_instance(const std::string &url);

  /*
   * Send NF instance registration to NRF
   * @param [const std::string &] url: nssf's URL
   * @param [nlohmann::json &] data: Json data to be sent
   * @return void
   */
  void send_update_nf_instance(const std::string &url,
                               const nlohmann::json &data);

  /*
   * Send NF deregister to nssf
   * @param [const std::string &] url: nssf's URL
   * @return void
   */
  void send_deregister_nf_instance(const std::string &url);

  /*
   * Trigger NF instance registration to NRF
   * @param [void]
   * @return void
   */
  void register_to_nrf();

  /*
   * Generate a random UUID for NSSF instance
   * @param [void]
   * @return void
   */
  void generate_uuid();

  /*
   * Generate a NSSF profile for this instance
   * @param [void]
   * @return void
   */
  void generate_nssf_profile();

  /*
   * will be executed when nssf Heartbeat timer expires
   * @param [timer_id_t] timer_id
   * @param [uint64_t] arg2_user
   * @return void
   */
  void timer_nssf_heartbeat_timeout(timer_id_t timer_id, uint64_t arg2_user);

  /*
   * will be executed when nssf Heartbeat timer expires
   * @param [timer_id_t] timer_id
   * @param [uint64_t] arg2_user
   * @return void
   */
  void timer_nssf_deregistration(timer_id_t timer_id, uint64_t arg2_user);

  /*
   * Send Curl command
   * @param [const std::string&] url: request url
   * @param [const std::string&] method: HTTP method
   * @param [std::string&] response: response from server
   * @param [uint32_t&] http_code: response code
   * @param [const std::string&] body: request body
   * @return void
   */
  void send_curl(const std::string &url, const std::string &method,
                 std::string &response, uint32_t &http_code,
                 const std::string &body = "");

  /*
   * Get nssf API Root
   * @param [std::string& ] api_root: nssf's API Root
   * @return void
   */
  void get_nssf_api_root(std::string &api_root);
};
} // namespace nssf
#endif /* FILEnssfnssf_HPP_SEEN */
