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

/*! \file nssf_slice_select.cpp
 \brief
 \author  Rohan Kharade
 \company Openairinterface Software Allianse
 \date Jan 2022
 \email: rohan.kharade@openairinterface.org
 */

#include "nssf_slice_selection.hpp"
#include "NetworkSliceInformationDocumentApiImpl.h"
#include "conversions.hpp"
#include "logger.hpp"
#include "nssf.h"
#include "nssf_config.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>

using namespace nssf;
using namespace std;

extern nssf_slice_select *nssf_slice_select_inst;
extern nssf_config nssf_cfg;

//------------------------------------------------------------------------------
bool nssf_slice_select::validate_nsi(const SliceInfoForPDUSession &slice_info,
                                     NsiInformation &nsi_info) {
  Logger::nssf_app().debug("Validating S-NSSAI for NSI");

  Snssai requested_snssai = slice_info.getSNssai();

  for (int i = 0; i < nssf_cfg.nssf_nsi_info.nsi_info_list.size(); i++) {
    Snssai target_snssai = nssf_cfg.nssf_nsi_info.nsi_info_list[i].snssai;

    if (requested_snssai.getSst() == target_snssai.getSst()) {
      if (requested_snssai.sdIsSet() & target_snssai.sdIsSet()) {
        if (requested_snssai.getSd() != target_snssai.getSd())
          return false;
      }

      nsi_info.setNrfId(
          nssf_cfg.nssf_nsi_info.nsi_info_list[i].nsi_info.getNrfId());

      if (nssf_cfg.nssf_nsi_info.nsi_info_list[i].nsi_info.nsiIdIsSet())
        nsi_info.setNsiId(
            nssf_cfg.nssf_nsi_info.nsi_info_list[i].nsi_info.getNsiId());

      if (nssf_cfg.nssf_nsi_info.nsi_info_list[i].nsi_info.nrfNfMgtUriIsSet())
        nsi_info.setNrfNfMgtUri(
            nssf_cfg.nssf_nsi_info.nsi_info_list[i].nsi_info.getNrfNfMgtUri());

      return true;
    }
  }

  Logger::nssf_app().warn("NS Selection: S-NSSAI from SliceInfoForPDUSession "
                          "is not authorised !!!");
  Logger::nssf_app().info(
      "//---------------------------------------------------------");
  Logger::nssf_app().info("");
  return false;
}
//------------------------------------------------------------------------------

bool nssf_slice_select::validate_ta(const Tai &tai) {
  Logger::nssf_app().debug("Validating TA");
  PlmnId requested_plmn = tai.getPlmnId();
  std::string requested_tac = tai.getTac();

  for (int i = 0; i < nssf_cfg.nssf_ta_info.ta_info_list.size(); i++) {
    PlmnId target_plmn = nssf_cfg.nssf_ta_info.ta_info_list[i].tai.getPlmnId();
    std::string target_tac = nssf_cfg.nssf_ta_info.ta_info_list[i].tai.getTac();

    if (requested_plmn.getMcc() == target_plmn.getMcc() &&
        requested_plmn.getMnc() == target_plmn.getMnc() &&
        requested_tac == target_tac)
      return true;
  }
  Logger::nssf_app().warn("NS Selection: TAI is not authorised !!!");
  Logger::nssf_app().info(
      "//---------------------------------------------------------");
  Logger::nssf_app().info("");
  return false;
}

//------------------------------------------------------------------------------
bool nssf_slice_select::handle_slice_info_for_registration(
    const SliceInfoForRegistration &slice_info, const Tai &tai,
    const PlmnId &home_plmnid, const std::string &features, int &http_code,
    const uint8_t http_version, const ProblemDetails &problem_details,
    AuthorizedNetworkSliceInfo &auth_slice_info) {
  Logger::nssf_app().info(
      "NS Selection: Handle case - Registration (HTTP_VERSION %d)",
      http_version);

  //### Step 1. Validation for roaming/EPS to 5GS Mobility procedure from
  // slice_info
  if (slice_info.requestMappingIsSet()) {
    Logger::nssf_app().debug("NS Selection: Request Mapping is provided");
    // Ts 29.531, R16.0.0, Table 6.1.6.2.10-1 SliceInfoForRegistration
    // This IE may be present when the Nnssf_NSSelection_Get procedure is
    // invoked during EPS to 5GS Mobility Registration Procedure (Idle and
    // Connected State) using N26 interface. When present this IE shall indicate
    // to the NSSF that the NSSF shall return the VPLMN specific mapped SNSSAI
    // values for the S-NSSAI values in the subscribedNssai IE.
    if (slice_info.sNssaiForMappingIsSet())
      ; // Ignore for now
    http_code = HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE;
    Logger::nssf_app().warn(
        "NS Selection: EPS to 5GS Mobility Registration Procedure is not "
        "Supported yet. "
        "RequestMapping can not be validated !!");
    Logger::nssf_app().info(
        "//---------------------------------------------------------");
    Logger::nssf_app().info("");
    return false;
  }

  if (slice_info.mappingOfNssaiIsSet()) {
    Logger::nssf_app().debug("NS Selection: Mapping of NSSAI is provided");
    // Ts 29.531, R16.0.0, Table 6.1.6.2.10-1 SliceInfoForRegistration
    // This IE shall contain the  mapping of S-NSSAI of the VPLMN to
    // corresponding HPLMN S-NSSAI, for the S-NSSAIs included in the
    // requestedNssai and allowedNssai IEs for the current and other
    // access types. It is also present in  EPS to 5GS handover procedure.
    http_code = HTTP_STATUS_CODE_503_SERVICE_UNAVAILABLE;
    Logger::nssf_app().warn(
        "NS Selection: Roaming/EPS to 5GS handover procedure is not "
        "Supported yet. "
        "RequestMapping can not be validated !!");
    Logger::nssf_app().info(
        "//---------------------------------------------------------");
    Logger::nssf_app().info("");
    return false;
  }

  //### Step 2. Validation for roaming from homeplmnid if provided
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
    return false;
  }

  //### Step 3. Validation for TAI if provided
  // Check if UE's Tai is Supported
  if (!tai.getTac().empty()) {
    Logger::nssf_app().debug("NS Selection: TAI is provided");
    if (!validate_ta(tai)) {
      http_code = HTTP_STATUS_CODE_400_BAD_REQUEST;
      return false;
    }
  }

  bool configure_nssai = false;
  //### Step 4. Validation of slice_info for target AMFSet
  // Check for Requested S-NSSAI
  if (slice_info.requestedNssaiIsSet()) {
    Logger::nssf_app().debug("NS Selection: Requested S-NSSAI is provided");
    // Step 4.1. Validate if Requested S-NSSAI is supported in PLMN
    // Else RejectedNssaiInPLMN

    // ToDo: 23.501, R16.0.0., 5.15.5.2.1 - If Requested NSSAI is not valid in
    // the Serving PLMN, then NSSF based on the Subscribed SNSSAI(s) and
    // operator configuration may also determine the Configured NSSAI

    // Step 4.2. Validate if Requested S-NSSAI is supported in Subscription
    if (slice_info.subscribedNssaiIsSet()) {
      Logger::nssf_app().debug("NS Selection: Subscribed S-NSSAI is provided");

      // Step 4.3. Validate if Requested S-NSSAI is supported in TA
      // Else RejectedNssaiInTA
    }
  }

  if (slice_info.allowedNssaiCurrentAccessIsSet()) {
    Logger::nssf_app().debug(
        "NS Selection: AllowedNssai Current Access is provided");
    // ToDo
  }

  if (slice_info.allowedNssaiOtherAccessIsSet()) {
    Logger::nssf_app().debug(
        "NS Selection: AllowedNssai Other Access is provided");
    // ToDo
  }

  if (slice_info.defaultConfiguredSnssaiIndIsSet()) {
    Logger::nssf_app().debug(
        "NS Selection: Default Configured NSSAI Indication is provided");
    // ToDo
  }
  return true;
}
//------------------------------------------------------------------------------
bool nssf_slice_select::handle_slice_info_for_pdu_session(
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
    return false;
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
    return false;
  }

  // Check if UE's Tai is Supported
  if (!tai.getTac().empty()) {
    Logger::nssf_app().debug("NS Selection: TAI is provided");
    if (!validate_ta(tai)) {
      http_code = HTTP_STATUS_CODE_400_BAD_REQUEST;
      return false;
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
  if (validate_nsi(slice_info, nsi_info)) {
    auth_slice_info.setNsiInformation(nsi_info);
    http_code = HTTP_STATUS_CODE_200_OK;
    return true;
  }
  http_code = HTTP_STATUS_CODE_400_BAD_REQUEST;
  return false;
}

//------------------------------------------------------------------------------
nssf_slice_select::~nssf_slice_select() {
  Logger::nssf_app().debug("Delete NSSF_SLICE_SELECT instance...");
}
