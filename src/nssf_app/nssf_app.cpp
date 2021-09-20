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

/*! \file nssf_app.cpp
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/
#include "nssf_app.hpp"
#include "conversions.hpp"
#include "itti.hpp"
#include "logger.hpp"
#include "nssf.h"
#include "nssf_config.hpp"
#include "nssf_sbi.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>

// #include "NFInstanceIDDocumentApiImpl.h"
// #include "NSSAIAvailabilityStoreApiImpl.h"
#include "NetworkSliceInformationDocumentApiImpl.h"
// #include "SubscriptionsCollectionApiImpl.h"

using namespace nssf;
using namespace std;

// C includes

nssf_nrf *nssf_nrf_inst = nullptr;

extern itti_mw *itti_inst;
extern nssf_app *nssf_app_inst;
extern nssf_config nssf_cfg;

void nssf_app_task(void *);

//------------------------------------------------------------------------------
void nssf_app_task(void *args_p) {
  const task_id_t task_id = TASK_NSSF_APP;

  const util::thread_sched_params *const sched_params =
      (const util::thread_sched_params *const)args_p;

  sched_params->apply(task_id, Logger::nssf_app());

  itti_inst->notify_task_ready(task_id);
  Logger::nssf_app().info("NSSF_APP TASK Created");
}

//------------------------------------------------------------------------------
nssf_app::nssf_app(const std::string &config_file) {
  Logger::nssf_app().startup("Starting...");
  nssf_cfg.execute();

  if (itti_inst->create_task(TASK_NSSF_APP, nssf_app_task,
                             &nssf_cfg.itti.nssf_app_sched_params)) {
    Logger::nssf_app().error("Cannot create task TASK_NSSF_APP");
    throw std::runtime_error("Cannot create task TASK_NSSF_APP");
  }
  try {
    if (nssf_cfg.nssf_features.register_nrf) {
      nssf_nrf_inst = new nssf_nrf();
      Logger::nssf_sbi().info("NRF TASK Created ");
    }
  } catch (std::exception &e) {
    Logger::nssf_sbi().error("Cannot create task NRF: %s", e.what());
    throw;
  }
  Logger::nssf_app().startup("Started");
}

//------------------------------------------------------------------------------
void nssf_app::handle_slice_info_for_registration(
    const SliceInfoForRegistration &slice_info, const Tai &tai,
    const PlmnId &home_plmnid, const std::string &features, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details) {
  Logger::nssf_app().info(
      "NS Selection: Handle case - Registration (Not Supported)");
  // ToDo
}
//------------------------------------------------------------------------------
void nssf_app::handle_slice_info_for_pdu_session(
    const SliceInfoForPDUSession &slice_info, const Tai &tai,
    const PlmnId &home_plmnid, const std::string &features, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details,
    AuthorizedNetworkSliceInfo &auth_slice_info) {
  Logger::nssf_app().info(
      "NS Selection: Handle case - PDU Session (HTTP_VERSION %d)",
      http_version);

  // Check if UE is Roamer
  RoamingIndication roam_ind = slice_info.getRoamingIndication();
  RoamingIndication_anyOf::eRoamingIndication_anyOf roam_ind_enum =
      roam_ind.getEnumValue();
  if (int(roam_ind_enum) != ROAMING_IND_NON_ROAMING) {
    Logger::nssf_app().warn(
        "NS Selection: Roming/Local Breakout is not Supported yet !!!");
    http_code = HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE;
    Logger::nssf_app().info(
        "//---------------------------------------------------------");
    Logger::nssf_app().info("");
    return;
  }

  // Check if UE's HPlmnId is Supported while UE is Roaminng
  if (!home_plmnid.getMcc().empty()) {
    Logger::nssf_app().debug("NS Selection: HomePlmnId is provided !!!");
    // ToDo - Validate PlmnId from nssf config (Currently we don't support
    // Roaming scenario)
    http_code = HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE;
    Logger::nssf_app().warn("NS Selection: Roming is not Supported yet. "
                            "HomePlmnId can not be validated !!");
    Logger::nssf_app().info(
        "//---------------------------------------------------------");
    Logger::nssf_app().info("");
    return;
  }

  // Check if UE's Tai is Supported
  if (!tai.getTac().empty()) {
    Logger::nssf_app().debug("NS Selection: TAI is provided");
    if (!nssf_cfg.ValidateTA(tai)) {
      http_code = HTTP_STATUS_CODE_400_BAD_REQUEST;
      return;
    }
  }

  // Check if Supported feature is Supported
  if (!features.empty()) {
    Logger::nssf_app().debug("NS Selection: features are provided");
    // ToDo
    // auth_slice_info.setSupportedFeatures("123abc");
  }

  // Check NSI info for given S-NSSAI is can be provided
  NsiInformation nsi_info = {};
  if (nssf_cfg.ValidateNSI(slice_info, nsi_info)) {
    auth_slice_info.setNsiInformation(nsi_info);
    http_code = HTTP_STATUS_CODE_200_OK;
    Logger::nssf_app().info(
        "NS Selection: Authorized Network Slice Info Returned !!!");
    Logger::nssf_app().info(
        "//---------------------------------------------------------");
    Logger::nssf_app().info("");
    return;
  }
  http_code = HTTP_STATUS_CODE_400_BAD_REQUEST;
  return;
}

//------------------------------------------------------------------------------
nssf_app::~nssf_app() {
  Logger::nssf_app().debug("Delete NSSF_APP instance...");
  if (nssf_nrf_inst)
    delete nssf_nrf_inst;
}
