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

#include <stdexec/exec/linux/safe_file_descriptor.hpp>
namespace exio {
namespace posix {
template <bool IsStream, bool IsFile, bool IsSocket> struct file_handle {
  constexpr static bool is_stream = IsStream;
  constexpr static bool is_file = IsFile;
  constexpr static bool is_socket = IsSocket;

  exec::safe_file_descriptor fd;
};
} // namespace posix
} // namespace exio
