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
#include "file/types.hpp"

namespace exio {
template <stream_io_scheduler Scheduler>
auto async_read_some(Scheduler const &sch, stream_handle_t &handle,
                     std::span<std::byte> buffer) {
  return sch.async_read_some(handle, buffer);
}
} // namespace exio
