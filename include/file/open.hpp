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
#include <filesystem>

#ifdef EXIO_POSIX
#include "posix/file/open.hpp"
#endif

namespace exio {
struct open_stream_t {
  template <typename... PlatformDependentArgs>
  auto operator()(std::filesystem::path const &path, open_flags_t flags,
                  PlatformDependentArgs &&...args) const {
#ifdef EXIO_POSIX
    return posix::open_stream(path, flags, args...);
#endif
  }
};

constexpr open_stream_t open_stream{};

struct open_file_t {
  template <typename... PlatformDependentArgs>
  auto operator()(std::filesystem::path const &path, open_flags_t flags,
                  PlatformDependentArgs &&...args) const {
#ifdef EXIO_POSIX
    return posix::open_file(path, flags, args...);
#endif
  }
};

constexpr open_file_t open_file{};
} // namespace exio
