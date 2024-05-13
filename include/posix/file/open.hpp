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

// TODO: can we make open_flags_t a template parameter?
//       (with supporting concepts)
inline auto open(std::filesystem::path const &path, open_flags_t flags) {
  auto result = ::open(path.c_str(), flags | O_CLOEXEC);
  if (result < 0) {
    throw_(std::system_error{errno, std::system_category()});
  }

  return exio::posix::file_handle<false>{exec::safe_file_descriptor{result}};
}
} // namespace posix
} // namespace exio
