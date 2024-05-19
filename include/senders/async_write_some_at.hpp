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

#include "file/concepts.hpp"

namespace exio {
struct async_write_some_at_t {
  template <stream_io_scheduler Scheduler, typename handle_t>
    requires(std::same_as<handle_t, stream_handle_t> ||
             std::same_as<handle_t, file_handle_t>)
  auto operator()(Scheduler const &sch, handle_t &handle, std::size_t offset,
                  std::span<std::byte const> buffer) const {
    return sch.async_write_some_at(handle, offset, buffer);
  }
};
constexpr async_write_some_at_t async_write_some_at{};
} // namespace exio
