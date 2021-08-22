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

/*! \file nssf_app.hpp
   \author  Lionel GAUTHIER
   \date 2018
   \email: lionel.gauthier@eurecom.fr
*/

#ifndef FILE_nssf_app_HPP_SEEN
#define FILE_nssf_app_HPP_SEEN

#include "common_root_types.h"

#include <boost/atomic.hpp>

#include <map>
#include <string>
#include <thread>
#include <memory>
#include <map>
#include <set>

#include "nssf_profile.hpp"


namespace nssf {

class nssf_app {
 private:
  std::thread::id thread_id;
  std::thread thread;

 public:
  explicit nssf_app(const std::string& config_file);
  nssf_app(nssf_app const&) = delete;
   void operator=(nssf_app const&) = delete;

  virtual ~nssf_app();

};
}  
#endif /* FILE_NSSF_APP_HPP_SEEN */
