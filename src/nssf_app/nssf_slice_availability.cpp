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

/*! \file nssf_slice_availability.cpp
 \brief
 \author  Rohan Kharade
 \company Openairinterface Software Allianse
 \date Jan 2022
 \email: rohan.kharade@openairinterface.org
 */

#include "nssf_slice_availability.hpp"
#include "conversions.hpp"
#include "logger.hpp"
#include "nssf.h"
#include "nssf_config.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>

using namespace nssf;
using namespace std;

extern nssf_slice_avail *nssf_slice_avail_inst;
extern nssf_config nssf_cfg;

//------------------------------------------------------------------------------
bool nssf_slice_avail::handle_create_nssai_availability_handler(
    const std::string &nfId,
    const SupportedNssaiAvailabilityData &nssaiAvailInfo, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details) {
  // ToDo
  // Tai tai                            = nssaiAvailInfo.getTai();
  // std::vector<ExtSnssai> snssai_list = nssaiAvai
  // lInfo.getSupportedSnssaiList(); if (nssaiAvailInfo.taiRangeListIsSet()) {
  // }
  // if (nssaiAvailInfo.taiListIsSet()) {
  // }
  return false;
}
//------------------------------------------------------------------------------
bool nssf_slice_avail::handle_update_nssai_availability_handler(
    const std::string &nfId, const std::vector<PatchItem> &patchItem,
    int &http_code, const uint8_t http_version,
    const ProblemDetails &problem_details) {
  // ToDo
  return false;
}
//------------------------------------------------------------------------------
bool nssf_slice_avail::handle_remove_nssai_availability_handler(
    const std::string &nfId, int &http_code, const uint8_t http_version,
    const ProblemDetails &problem_details) {
  // ToDo
  return false;
}
//------------------------------------------------------------------------------
// Handle NSSF NSSAI Availability - Subscription ID (Collection/Document)
bool nssf_slice_avail::handle_create_subscription_nssai_availability_handler(
    const NssfEventSubscriptionCreateData &subscriptionData, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details) {
  // ToDo
  return false;
}
//------------------------------------------------------------------------------
bool nssf_slice_avail::handle_update_subscription_nssai_availability_handler(
    const NssfEventSubscriptionCreateData &subscriptionData, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details) {
  // ToDo
  return false;
}
//------------------------------------------------------------------------------
bool nssf_slice_avail::handle_remove_subscription_nssai_availability_handler(
    const std::string &subscriptionId, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details) {
  // ToDo
  return false;
}
//------------------------------------------------------------------------------
nssf_slice_avail::~nssf_slice_avail() {
  Logger::nssf_app().debug("Delete NSSF_SLICE_AVAIL instance...");
}
