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
#include "unix/file_handle.hpp"
#include <fcntl.h>
#include <filesystem>

namespace exio {
inline auto open_read_only(std::filesystem::path const &path) {
  auto result = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (result < 0) {
    throw_(std::system_error{errno, std::system_category()});
  }

  return exio::file_handle<open_mode::READ_ONLY>{
      exec::safe_file_descriptor{result}};
}
} // namespace exio
