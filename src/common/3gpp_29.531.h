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

#ifndef FILE_3GPP_29_531_SEEN
#define FILE_3GPP_29_531_SEEN

#include <vector>
#include <stdint.h>
#include <string>
#include "3gpp_23.003.h"
#include "3gpp_29.571.h"
#include "3gpp_29.510.h"

//-------------------------------------
// 6.1.6.2.2 Authorized Network Slice Info
// typedef struct authorized_network_slice_info_s {
// ToDo in src/nssf_*.cpp
// } authorized_network_slice_info_t;

//-------------------------------------
// 6.1.6.2.3 Subscribed Snssai
typedef struct subscribed_snssai_s {
snssai_t subscribed_snssai;
bool default_indication;
} subscribed_snssai_t;

//-------------------------------------
// 6.1.6.2.7 Nsi Information
typedef struct nsi_information_s {
std::string nrf_id;
std::string nsi_id;
std::string nrf_nfmgt_uri;
std::string nrf_access_token_uri;
} nsi_information_t;

//-------------------------------------
// 6.1.6.2.5 Allowed Snssai
typedef struct allowed_snssai_s {
snssai_t allowed_snssai;
std::vector<nsi_information_t> nsi_info_list;
snssai_t mapped_home_snssai;
} allowed_snssai_t;

//-------------------------------------
// 6.1.6.2.6 Allowed Nssai
typedef struct allowed_nssai_s {
uint8_t access_type;
std::vector<allowed_snssai_t> allowed_snssai_list;
} allowed_nssai_t;

//-------------------------------------
// 6.1.6.2.8 Mapping Of Snssai
typedef struct mapping_of_snssai_s {
snssai_t serving_snssai;
snssai_t home_snssai;
} mapping_of_snssai_t;

//-------------------------------------
// 6.1.6.2.10 Slice Info For Registration
// Contains the slice information requested during Registration procedure. 
// typedef struct slice_info_for_registration_s {
// ToDo in src/nssf_*.cpp
// } slice_info_for_registration_t;

//-------------------------------------
// 6.1.6.2.11 Slice Info For PDU Session
// typedef struct slice_info_for_pdu_session_s {
// snssai_t snssai;
// std::string roaming_indication;
// snssai_t home_snssai;
// } slice_info_for_pdu_session_t;

//-------------------------------------
// 6.1.6.2.12 Configured Snssai
typedef struct configured_snssai_s {
snssai_t configured_snssai;
snssai_t mapped_home_snssai;
} configured_snssai_t;

//-------------------------------------
// 6.1.6.2.13 Slice Info For UE Configuration Update
// typedef struct slice_info_for_ue_config_update_s {
// std::vector <subscribed_snssai_t> subscribed_snssai;
// allowed_nssai_t allowed_nssai_current_access;
// allowed_nssai_t allowed_nssai_other_access;
// bool default_configured_snssai_ind;
// std::vector<snssai_t> requested_nssai;
// std::vector<mapping_of_snssai_t> mapping_of_nssai;
// } slice_info_for_ue_config_update_t;
//################

// 6.1.6.3.3 Enumeration Roaming Indication
typedef enum roaming_indication_s {
ROAMING_INDICATION_NON_ROAMING         = 1,
ROAMING_INDICATION_LOCAL_BREAKOUT      = 2,
ROAMING_INDICATION_HOME_ROUTED_ROAMING = 3
} roaming_indication_t;

static const std::vector<std::string> roaming_indication_e2str = {
    "ROAMING_INDICATION_NON_ROAMING", "ROAMING_INDICATION_LOCAL_BREAKOUT", 
    "ROAMING_INDICATION_HOME_ROUTED_ROAMING"};

//################
//-------------------------------------
// 6.2.6.2.3 Supported Nssai Availability Data
typedef struct supported_nssai_availability_data_s {
tai_t tai;
std::vector<ext_snssai_t> supported_snssai_list;
std::vector<tai_t> tai_list;
std::vector<tai_range_t> tai_range_list;
} supported_nssai_availability_data_t;

//-------------------------------------
// 6.2.6.2.2 Nssai Availability Info
typedef struct nssai_availability_info_s {
std::vector<supported_nssai_availability_data_t> supported_nssai_availability_data;
std::string supported_features;
std::string amf_set_id;
} nssai_availability_info_t;

//-------------------------------------
// 6.2.6.2.5 Restricted Snssai
typedef struct restricted_snssai_s {
plmn_t home_plmn_id;
std::vector<snssai_t> sNssaiList;
std::vector<plmn_t> home_plmn_id_list;
bool roaming_restriction;
} restricted_snssai_t;

//-------------------------------------
// 6.2.6.2.4 Authorized Nssai Availability Data
typedef struct authorized_nssai_availability_data_s {
tai_t tai;
std::vector<ext_snssai_t> supported_snssai_list;
std::vector<restricted_snssai_t> restricted_snssai_list;
std::vector<tai_t> tai_list;
std::vector<tai_range_t> tai_range_list;
} authorized_nssai_availability_data_t;

//-------------------------------------
// 6.2.6.2.6 Authorized Nssai Availability Info
typedef struct authorized_nssai_availability_info_s {
std::vector<authorized_nssai_availability_data_t> auth_nssai_availability_data;
std::string supported_features;
} authorized_nssai_availability_info_t;

//-------------------------------------
// 6.2.6.2.7

//-------------------------------------
// 6.2.6.2.8 Nssf Event Subscription Create Data
typedef struct nssf_event_subscription_create_data_s {
std::string nf_nssai_availability_uri;
std::vector<tai_t> tai_list;
std::string event; //enum NssfEventType
std::string expiry; //as per DateTime in TS 29.571 
std::string amf_set_id;
} nssf_event_subscription_create_data_t;

//-------------------------------------
// 6.2.6.2.9 Nssf Event Subscription Created Data 
typedef struct nssf_event_subscription_created_data_s {
std::string subscription_id;
std::string expiry; //as per DateTime in TS 29.571 
std::vector<authorized_nssai_availability_data_t> auth_nssai_avail_data;
} nssf_event_subscription_created_data_t;

//-------------------------------------
// 6.2.6.2.10 Nssf Event Notification
typedef struct nssf_event_notification_s {
  std::string subscription_id;
  std::vector<authorized_nssai_availability_data_t> auth_nssai_avail_data;
} nssf_event_notification_t;

//################
// 6.2.6.3.3 Enumeration: Nssf Event Type
enum nssf_event_type_e {
SNSSAI_STATUS_CHANGE_REPORT
};

static const std::vector<std::string> nssf_event_type_e2str = {
    "SNSSAI_STATUS_CHANGE_REPORT"};

#endif
