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
namespace __file_details {
template <typename Scheduler, typename Stream>
concept __stream_io_scheduler =
    stdexec::scheduler<Scheduler> &&
    requires(Scheduler const &sch, Stream &handle,
             std::span<std::byte> mutable_buffer,
             std::span<std::byte const> buffer) {
      {
        sch.async_read_some(handle, mutable_buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
      {
        sch.async_write_some(handle, buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
    };

template <typename Scheduler, typename File>
concept __file_io_scheduler =
    __stream_io_scheduler<Scheduler, File> &&
    requires(Scheduler const &sch, File &handle,
             std::span<std::byte> mutable_buffer,
             std::span<std::byte const> buffer, std::size_t offset) {
      {
        sch.async_read_some_at(handle, offset, mutable_buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
      {
        sch.async_write_some_at(handle, offset, buffer)
      } -> stdexec::sender_of<stdexec::set_value_t(std::size_t)>;
    };
} // namespace __file_details
template <typename Scheduler>
concept stream_io_scheduler =
    __file_details::__stream_io_scheduler<Scheduler, stream_handle_t>;

template <typename Scheduler>
concept file_io_scheduler =
    __file_details::__file_io_scheduler<Scheduler, file_handle_t>;
} // namespace exio
