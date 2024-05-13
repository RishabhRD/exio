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

#include "config.hpp"
#include "file/open_flags.hpp"
#include "io_uring/io_uring_context.hpp"
#include <filesystem>
#include <stdexec/stdexec/execution.hpp>
#ifdef EXIO_POSIX
#include "posix/file/open.hpp"
#endif
namespace exio {
template <typename T>
// TODO: Define clearly what an IO Context is
concept IOContext = requires(T &ctx) {
  { ctx.get_scheduler() } -> stdexec::scheduler;
};

using io_context = exio::io_uring_context;

inline auto open(std::filesystem::path const &path, open_flags_t flags) {
#ifdef EXIO_POSIX
  return posix::open(path, flags);
#endif
}
} // namespace exio
