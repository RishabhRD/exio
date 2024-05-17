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

#include "exception.hpp"
#include "fcntl.h"
#include "open_flags.hpp"
#include "posix/file/file_handle.hpp"
#include <filesystem>

namespace exio {
namespace posix {

namespace __posix_open_details {
inline auto open(std::string_view path, open_flags_t flags,
                 int permission = 0644) {
  auto result = ::open(path.data(), flags | O_CLOEXEC, permission);
  if (result < 0) {
    throw_(std::system_error{errno, std::system_category()});
  }

  return exec::safe_file_descriptor{result};
}
} // namespace __posix_open_details

inline auto open_stream(std::string_view path, open_flags_t flags,
                        int permission = 0644) {
  return exio::posix::file_handle<true, false, false>{
      __posix_open_details::open(path, flags, permission)};
}

inline auto open_file(std::filesystem::path const &path, open_flags_t flags,
                      int permission = 0644) {
  return exio::posix::file_handle<true, true, false>{
      __posix_open_details::open(path.c_str(), flags, permission)};
}
} // namespace posix
} // namespace exio
