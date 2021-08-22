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

/*! \file nssf_config.cpp
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/

#include "nssf_config.hpp"
#include "async_shell_cmd.hpp"
#include "common_defs.h"
#include "conversions.hpp"
#include "fqdn.hpp"
#include "get_gateway_netlink.hpp"
#include "if.hpp"
#include "logger.hpp"
#include "string.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace libconfig;
using namespace nssf;

// C includes
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//------------------------------------------------------------------------------
int nssf_config::execute() { return RETURNok; }

//------------------------------------------------------------------------------

int nssf_config::load_interface(const Setting &if_cfg, interface_cfg_t &cfg) {
  if_cfg.lookupValue(NSSF_CONFIG_STRING_INTERFACE_NAME, cfg.if_name);
  util::trim(cfg.if_name);
  if (not boost::iequals(cfg.if_name, "none")) {
    std::string address = {};
    if_cfg.lookupValue(NSSF_CONFIG_STRING_IPV4_ADDRESS, address);
    util::trim(address);
    if (boost::iequals(address, "read")) {
      if (get_inet_addr_infos_from_iface(cfg.if_name, cfg.addr4, cfg.network4,
                                         cfg.mtu)) {
        Logger::nssf_app().error(
            "Could not read %s network interface configuration", cfg.if_name);
        return RETURNerror;
      }
    } else {
      std::vector<std::string> words;
      boost::split(words, address, boost::is_any_of("/"),
                   boost::token_compress_on);
      if (words.size() != 2) {
        Logger::nssf_app().error("Bad value " NSSF_CONFIG_STRING_IPV4_ADDRESS
                                 " = %s in config file",
                                 address.c_str());
        return RETURNerror;
      }
      unsigned char buf_in_addr[sizeof(struct in6_addr)]; // you never know...
      if (inet_pton(AF_INET, util::trim(words.at(0)).c_str(), buf_in_addr) ==
          1) {
        memcpy(&cfg.addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::nssf_app().error(
            "In conversion: Bad value " NSSF_CONFIG_STRING_IPV4_ADDRESS
            " = %s in config file",
            util::trim(words.at(0)).c_str());
        return RETURNerror;
      }
      cfg.network4.s_addr =
          htons(ntohs(cfg.addr4.s_addr) &
                0xFFFFFFFF << (32 - std::stoi(util::trim(words.at(1)))));
    }
    if_cfg.lookupValue(NSSF_CONFIG_STRING_SBI_PORT_HTTP1, cfg.http1_port);
    if_cfg.lookupValue(NSSF_CONFIG_STRING_SBI_PORT_HTTP2, cfg.http2_port);
  }
  return RETURNok;
}

//------------------------------------------------------------------------------
int nssf_config::load(const string &config_file) {
  Config cfg;
  unsigned char buf_in_addr[sizeof(struct in_addr) + 1];
  unsigned char buf_in6_addr[sizeof(struct in6_addr) + 1];

  // Read the file. If there is an error, report it and exit.
  try {
    cfg.readFile(config_file.c_str());
  } catch (const FileIOException &fioex) {
    Logger::nssf_app().error("I/O error while reading file %s - %s",
                             config_file.c_str(), fioex.what());
    throw;
  } catch (const ParseException &pex) {
    Logger::nssf_app().error("Parse error at %s:%d - %s", pex.getFile(),
                             pex.getLine(), pex.getError());
    throw;
  }

  const Setting &root = cfg.getRoot();

  try {
    const Setting &nssf_cfg = root[NSSF_CONFIG_STRING_NSSF_CONFIG];
  } catch (const SettingNotFoundException &nfex) {
    Logger::nssf_app().error("%s : %s", nfex.what(), nfex.getPath());
    return RETURNerror;
  }

  const Setting &nssf_cfg = root[NSSF_CONFIG_STRING_NSSF_CONFIG];

  try {
    nssf_cfg.lookupValue(NSSF_CONFIG_STRING_FQDN, fqdn);
    util::trim(fqdn);
  } catch (const SettingNotFoundException &nfex) {
    Logger::nssf_app().info("%s : %s, No FQDN configured", nfex.what(),
                            nfex.getPath());
  }

  try {
    const Setting &nw_if_cfg = nssf_cfg[NSSF_CONFIG_STRING_INTERFACES];

    const Setting &sbi_cfg = nw_if_cfg[NSSF_CONFIG_STRING_INTERFACE_SBI];
    load_interface(sbi_cfg, sbi);

    // Supported features
    const Setting &support_features =
        nssf_cfg[NSSF_CONFIG_STRING_SUPPORTED_FEATURES];

    string opt;
    support_features.lookupValue(NSSF_CONFIG_STRING_SUPPORTED_FEATURES_USE_FQDN,
                                 opt);
    if (boost::iequals(opt, "yes")) {
      nssf_features.use_fqdn = true;
    } else {
      nssf_features.use_fqdn = false;
    }

    support_features.lookupValue(
        NSSF_CONFIG_STRING_SUPPORTED_FEATURES_REGISTER_NRF, opt);
    if (boost::iequals(opt, "yes")) {
      nssf_features.register_nrf = true;
    } else {
      nssf_features.register_nrf = false;
    }

    // NRF
    const Setting &nrf_cfg =
        support_features[NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF];
    struct in_addr nssf_ipv4_addr;
    unsigned int nrf_port = 0;
    unsigned int httpVersion = 0;
    std::string nssf_api_version;
    string nrf_address = {};

    if (nssf_features.register_nrf) {
      if (!(nrf_cfg.lookupValue(
              NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_HTTP_VERSION,
              httpVersion))) {
        Logger::nssf_app().error(
            NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_HTTP_VERSION "failed");
        throw(NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_HTTP_VERSION "failed");
      }
      nssf_features.nrf_addr.http_version = httpVersion;

      if (!nssf_features.use_fqdn) {
        nrf_cfg.lookupValue(
            NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_IPV4_ADDRESS,
            nrf_address);
        IPV4_STR_ADDR_TO_INADDR(util::trim(nrf_address).c_str(), nssf_ipv4_addr,
                                "BAD IPv4 ADDRESS FORMAT FOR NSSF !");
        nssf_features.nrf_addr.ipv4_addr = nssf_ipv4_addr;
        if (!(nrf_cfg.lookupValue(
                NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_PORT, nrf_port))) {
          Logger::nssf_app().error(
              NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_PORT "failed");
          throw(NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_PORT "failed");
        }
        nssf_features.nrf_addr.port = nrf_port;

        if (!(nrf_cfg.lookupValue(
                NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_API_VERSION,
                nssf_api_version))) {
          Logger::nssf_app().error(
              NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_API_VERSION "failed");
          throw(NSSF_CONFIG_STRING_SUPPORTED_FEATURES_NRF_API_VERSION "failed");
        }
        nssf_features.nrf_addr.api_version = nssf_api_version;
      } else {
        Logger::nssf_app().info("USE FQDN");
        std::string nssf_fqdn = {};
        nrf_cfg.lookupValue(NSSF_CONFIG_STRING_FQDN, nssf_fqdn);
        nssf_features.nrf_addr.fqdn =
            nssf_fqdn; // TODO: Resolve FQDN at runtime
        uint8_t addr_type = {};
        fqdn::resolve(nssf_fqdn, nrf_address, nrf_port, addr_type);
        if (addr_type != 0) { // IPv6: TODO
          throw("DO NOT SUPPORT IPV6 ADDR FOR nssf!");
        } else { // IPv4
          IPV4_STR_ADDR_TO_INADDR(util::trim(nrf_address).c_str(),
                                  nssf_ipv4_addr,
                                  "BAD IPv4 ADDRESS FORMAT FOR nssf !");
          nssf_features.nrf_addr.ipv4_addr = nssf_ipv4_addr;
          nssf_features.nrf_addr.port = nrf_port;
          nssf_features.nrf_addr.api_version = "v1"; // TODO: get API version
        }
      }
    }
  } catch (const SettingNotFoundException &nfex) {
    Logger::nssf_app().error("%s : %s", nfex.what(), nfex.getPath());
    return RETURNerror;
  }
  return RETURNok;
}

//------------------------------------------------------------------------------
void nssf_config::display() {
  Logger::nssf_app().info("==== OPENAIRINTERFACE %s v%s ====", PACKAGE_NAME,
                          PACKAGE_VERSION);
  Logger::nssf_app().info("Configuration:");
  Logger::nssf_app().info("- FQDN ..................: %s", fqdn.c_str());
  Logger::nssf_app().info("- ITTI tasks:");
  Logger::nssf_app().info("    ITTI Timer task:");
  Logger::nssf_app().info("    NSSF task:");
  Logger::nssf_app().info("      CPU ID .........: %d",
                          itti.async_cmd_sched_params.cpu_id);
  Logger::nssf_app().info("      sched policy....: %d",
                          itti.async_cmd_sched_params.sched_policy);
  Logger::nssf_app().info("      sched priority..: %d",
                          itti.async_cmd_sched_params.sched_priority);
  Logger::nssf_app().info("- SBI:");
  Logger::nssf_app().info("    iface ............: %s", sbi.if_name.c_str());
  Logger::nssf_app().info("    ipv4.addr ........: %s", inet_ntoa(sbi.addr4));
  Logger::nssf_app().info("    ipv4.mask ........: %s",
                          inet_ntoa(sbi.network4));
  Logger::nssf_app().info("    mtu ..............: %d", sbi.mtu);
  Logger::nssf_app().info("    http1_port .......: %u", sbi.http1_port);
  Logger::nssf_app().info("    http2_port .......: %u", sbi.http2_port);
  Logger::nssf_app().info("    Reader thread:");
  Logger::nssf_app().info("      CPU ID .........: %d (TODO)",
                          sbi.thread_rd_sched_params.cpu_id);
  Logger::nssf_app().info("      sched policy....: %d (TODO)",
                          sbi.thread_rd_sched_params.sched_policy);
  Logger::nssf_app().info("      sched priority..: %d (TODO)",
                          sbi.thread_rd_sched_params.sched_priority);
  Logger::nssf_app().info("      thread pool size: %d (TODO)",
                          sbi.thread_rd_sched_params.thread_pool_size);
  Logger::nssf_app().info("- SUPPORTED_FEATURES:");

  Logger::nssf_app().info("    Register NRF: %s",
                          (nssf_features.register_nrf) ? "yes" : "no");

  Logger::nssf_app().info("    NRF:");
  Logger::nssf_app().info(
      "        IPv4 Addr .......: %s",
      inet_ntoa(*((struct in_addr *)&nssf_features.nrf_addr.ipv4_addr)));
  Logger::nssf_app().info("        Port ............: %lu  ",
                          nssf_features.nrf_addr.port);
  Logger::nssf_app().info("        HTTP Version ....: %lu  ",
                          nssf_features.nrf_addr.http_version);
  Logger::nssf_app().info("        API version .....: %s",
                          nssf_features.nrf_addr.api_version.c_str());
}
