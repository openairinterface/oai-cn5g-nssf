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

/*! \file nssf_nrf.cpp
 \brief
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#include "nssf_sbi.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "3gpp_29.500.h"
#include "3gpp_29.510.h"
#include "itti.hpp"
#include "logger.hpp"
#include "nssf_config.hpp"

using namespace nssf;
using json = nlohmann::json;

extern itti_mw *itti_inst;
extern nssf_nrf *nssf_nrf_inst;
extern nssf_config nssf_cfg;
void nssf_nrf_task(void *);

// To read content of the response from NF
static std::size_t callback(const char *in, std::size_t size, std::size_t num,
                            std::string *out) {
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}

//------------------------------------------------------------------------------
void nssf_nrf_task(void *args_p) {
  const task_id_t task_id = TASK_NSSF_NRF;
  itti_inst->notify_task_ready(task_id);

  do {
    std::shared_ptr<itti_msg> shared_msg = itti_inst->receive_msg(task_id);
    auto *msg = shared_msg.get();
    switch (msg->msg_type) {
    case TIME_OUT:
      if (itti_msg_timeout *to = dynamic_cast<itti_msg_timeout *>(msg)) {
        Logger::nssf_sbi().info("TIME-OUT event timer id %d", to->timer_id);
        switch (to->arg1_user) {
        case TASK_NSSF_SBI_TIMEOUT_NRF_HEARTBEAT:
          nssf_nrf_inst->timer_nssf_heartbeat_timeout(to->timer_id,
                                                      to->arg2_user);
          break;
        case TASK_NSSF_SBI_TIMEOUT_NRF_DEREGISTRATION:
          nssf_nrf_inst->timer_nssf_deregistration(to->timer_id, to->arg2_user);
          break;
        default:;
        }
      }
      break;

    case TERMINATE:
      if (itti_msg_terminate *terminate =
              dynamic_cast<itti_msg_terminate *>(msg)) {
        Logger::nssf_sbi().info("Received terminate message");
        return;
      }
      break;

    default:
      Logger::nssf_sbi().info("no handler for msg type %d", msg->msg_type);
    }

  } while (true);
}

//------------------------------------------------------------------------------
nssf_nrf::nssf_nrf() {
  Logger::nssf_sbi().startup("Starting...");

  nssf_nf_profile = {};
  nssf_instance_id = {};
  timer_nssf_heartbeat = {};

  if (itti_inst->create_task(TASK_NSSF_NRF, nssf_nrf_task, nullptr)) {
    Logger::nssf_sbi().error("Cannot create task TASK_NSSF_NRF");
    throw std::runtime_error("Cannot create task TASK_NSSF_NRF");
  }

  std::thread();

  // Register to NRF
  register_to_nrf();
  Logger::nssf_sbi().startup("Started");
}

//-----------------------------------------------------------------------------------------------------
void nssf_nrf::send_register_nf_instance(const std::string &url) {
  Logger::nssf_sbi().info("Send NF Instance Registration to NRF");

  nlohmann::json json_data = {};
  nssf_nf_profile.to_json(json_data);

  Logger::nssf_sbi().debug("Send NF Instance Registration to NRF (NRF URL %s)",
                           url.c_str());

  std::string body = json_data.dump();
  Logger::nssf_sbi().debug(
      "Send NF Instance Registration to NRF, msg body: \n %s", body.c_str());

  std::string response = {};
  uint32_t http_code = {0};
  send_curl(url, "PUT", response, http_code, body);

  if (http_code == HTTP_STATUS_CODE_201_CREATED) {
    json response_data = {};
    try {
      response_data = json::parse(response.c_str());
    } catch (json::exception &e) {
      Logger::nssf_sbi().warn("Could not parse JSON from the nrf response");
    }
    Logger::nssf_sbi().info("Response from nrf, JSON data: \n %s",
                            response_data.dump().c_str());

    // Update NF profile
    nssf_nf_profile.from_json(response_data);
    nssf_nf_profile.display();

    // Set Heartbeat timer timeout
    timer_nssf_heartbeat = itti_inst->timer_setup(
        nssf_nf_profile.get_nf_heartBeat_timer(), 0, TASK_NSSF_NRF,
        TASK_NSSF_SBI_TIMEOUT_NRF_HEARTBEAT,
        0); // TODO arg2_user

  } else {
    Logger::nssf_sbi().warn("Could not get response from NRF");
  }
}

//-----------------------------------------------------------------------------------------------------
void nssf_nrf::send_update_nf_instance(const std::string &url,
                                       const nlohmann::json &data) {
  Logger::nssf_sbi().info("Send NF Update to NRF");

  std::string body = data.dump();
  Logger::nssf_sbi().debug("Send NF Update to nssf (Msg body %s)",
                           body.c_str());

  Logger::nssf_sbi().debug("Send NF Update to nssf (nssf URL %s)", url.c_str());

  std::string response = {};
  uint32_t http_code = {0};
  send_curl(url, "PATCH", response, http_code, body);

  if ((http_code == HTTP_STATUS_CODE_200_OK) or
      (http_code == HTTP_STATUS_CODE_204_NO_CONTENT)) {
    Logger::nssf_sbi().info("Got successful response from nssf");
  } else {
    Logger::nssf_sbi().warn("Could not get response from nssf");
  }
}

//-----------------------------------------------------------------------------------------------------
void nssf_nrf::send_deregister_nf_instance(const std::string &url) {
  Logger::nssf_sbi().info("Send NF De-register to nssf");

  Logger::nssf_sbi().debug("Send NF De-register to nssf (nssf URL %s)",
                           url.c_str());

  std::string response = {};
  uint32_t http_code = {0};
  send_curl(url, "DELETE", response, http_code);

  // TODO:
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::generate_nssf_profile() {
  // generate UUID
  generate_uuid();
  // TODO: remove hardcoded values
  nssf_nf_profile.set_nf_instance_id(nssf_instance_id);
  nssf_nf_profile.set_nf_instance_name("OAI-NSSF");
  nssf_nf_profile.set_nf_type("NSSF");
  nssf_nf_profile.set_nf_status("REGISTERED");
  nssf_nf_profile.set_nf_heartBeat_timer(50);
  nssf_nf_profile.set_nf_priority(1);
  nssf_nf_profile.set_nf_capacity(100);
  nssf_nf_profile.set_fqdn(nssf_cfg.fqdn);
  nssf_nf_profile.add_nf_ipv4_addresses(nssf_cfg.sbi.addr4); // N4's Addr
  nssf_nf_profile.display();
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::register_to_nrf() {
  // Create a NF profile for this instance
  generate_nssf_profile();
  // Then register to NRF
  std::string nssf_api_root = {};
  get_nssf_api_root(nssf_api_root);
  send_register_nf_instance(nssf_api_root + NSSF_NF_REGISTER_URL +
                            nssf_instance_id);
}

//------------------------------------------------------------------------------
void nssf_nrf::generate_uuid() {
  nssf_instance_id = to_string(boost::uuids::random_generator()());
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::timer_nssf_heartbeat_timeout(timer_id_t timer_id,
                                            uint64_t arg2_user) {
  Logger::nssf_sbi().debug("Send Heartbeat to nssf");

  patch_item_t patch_item = {};
  //{"op":"replace","path":"/nfStatus", "value": "REGISTERED"}
  patch_item.op = "replace";
  patch_item.path = "/nfStatus";
  patch_item.value = "REGISTERED";

  nlohmann::json json_data = nlohmann::json::array();
  nlohmann::json item = patch_item.to_json();
  json_data.push_back(item);

  std::string nssf_api_root = {};
  get_nssf_api_root(nssf_api_root);

  Logger::nssf_sbi().debug("Set a timer to the next Heart-beat (%d)",
                           nssf_nf_profile.get_nf_heartBeat_timer());
  timer_nssf_heartbeat =
      itti_inst->timer_setup(nssf_nf_profile.get_nf_heartBeat_timer(), 0,
                             TASK_NSSF_NRF, TASK_NSSF_SBI_TIMEOUT_NRF_HEARTBEAT,
                             0); // TODO arg2_user

  send_update_nf_instance(
      nssf_api_root + NSSF_NF_REGISTER_URL + nssf_instance_id, json_data);
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::timer_nssf_deregistration(timer_id_t timer_id,
                                         uint64_t arg2_user) {
  Logger::nssf_sbi().debug("Send NF De-registration to NRF");

  std::string nssf_api_root = {};
  get_nssf_api_root(nssf_api_root);

  send_deregister_nf_instance(nssf_api_root + NSSF_NF_REGISTER_URL +
                              nssf_instance_id);
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::send_curl(const std::string &url, const std::string &method,
                         std::string &response, uint32_t &http_code,
                         const std::string &body) {
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl = curl_easy_init();

  if (curl) {
    CURLcode res = {};
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, NSSF_CURL_TIMEOUT_MS);
    curl_easy_setopt(
        curl, CURLOPT_INTERFACE,
        nssf_cfg.sbi.if_name.c_str()); // TODO: use another interface for NSSF
                                       // to communicate with NRF
    if (nssf_cfg.nssf_features.nrf_addr.http_version == 2) {
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      // We use a self-signed test server, skip verification during debugging
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,
                       CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    }

    // Response information
    long code = {0};
    std::unique_ptr<std::string> httpData(new std::string());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    if (body.length() > 0) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
      Logger::nssf_sbi().info("Response from nssf, HTTP Code: %ld", code);
      http_code = code;
      if (code != HTTP_STATUS_CODE_204_NO_CONTENT)
        response = *httpData.get();
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
}

//---------------------------------------------------------------------------------------------
void nssf_nrf::get_nssf_api_root(std::string &api_root) {
  api_root =
      std::string(inet_ntoa(
          *((struct in_addr *)&nssf_cfg.nssf_features.nrf_addr.ipv4_addr))) +
      ":" + std::to_string(nssf_cfg.nssf_features.nrf_addr.port) +
      NNRF_NFM_BASE + nssf_cfg.nssf_features.nrf_addr.api_version;
}
