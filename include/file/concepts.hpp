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

#include "file/types.hpp"
#include <span>
#include <stdexec/stdexec/execution.hpp>

namespace exio {
template <typename Scheduler>
concept stream_io_scheduler =
    stdexec::scheduler<Scheduler> &&
    requires(Scheduler const &sch, stream_handle_t &handle,
             std::span<std::byte> mutable_buffer,
             std::span<std::byte const> buffer) {
      {
        sch.async_read_some(handle, mutable_buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
      {
        sch.async_write_some(handle, buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
    };
} // namespace exio
