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
#include "conversions.hpp"
#include "itti.hpp"
#include "logger.hpp"
#include "nssf_app.hpp"
#include "nssf_config.hpp"
#include "nssf_sbi.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>

using namespace nssf;
using namespace std;

// C includes

nssf_nrf* nssf_nrf_inst = nullptr;

extern itti_mw* itti_inst;
extern nssf_app* nssf_app_inst;
extern nssf_config nssf_cfg;

void nssf_app_task(void*);

//------------------------------------------------------------------------------
void nssf_app_task(void* args_p) {
  const task_id_t task_id = TASK_NSSF_APP;

  const util::thread_sched_params* const sched_params =
      (const util::thread_sched_params* const) args_p;
  
  sched_params->apply(task_id, Logger::nssf_app());

  itti_inst->notify_task_ready(task_id);
  Logger::nssf_app().info("NSSF_APP TASK Created");
}

//------------------------------------------------------------------------------
nssf_app::nssf_app(const std::string& config_file) {
  Logger::nssf_app().startup("Starting...");
  nssf_cfg.execute();

  // if (itti_inst->create_task(
  //         TASK_NSSF_APP, nssf_app_task,
  //          &nssf_cfg.itti.nssf_app_sched_params)) {
  //   Logger::nssf_app().error("Cannot create task TASK_NSSF_APP");
  //   throw std::runtime_error("Cannot create task TASK_NSSF_APP");
  // }
  try {
    if (nssf_cfg.nssf_features.register_nrf) {
      nssf_nrf_inst = new nssf_nrf();
      Logger::nssf_sbi().info("NRF TASK Created ");
      }
  } catch (std::exception& e) {
    Logger::nssf_sbi().error("Cannot create task NRF: %s", e.what());
    throw;
  }
  Logger::nssf_app().startup("Started");
}

//------------------------------------------------------------------------------
nssf_app::~nssf_app() {
  if (nssf_nrf_inst) delete nssf_nrf_inst;
}
