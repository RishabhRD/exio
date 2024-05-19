/*
 * Copyright (c) 2024 Rishabh Dwivedi <rishabhdwivedi17@gmail.com>
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdexec/stdexec/execution.hpp>

namespace exio {
namespace __env_details {
using namespace stdexec::tags;
template <typename Scheduler> struct env_t {
  Scheduler sch;

  friend auto
  tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>,
             env_t const &env) noexcept -> Scheduler {
    return env.sch;
  }
};
} // namespace __env_details
} // namespace exio
