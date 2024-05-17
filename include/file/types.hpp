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

#include "file/open.hpp"
#include "file/open_flags.hpp"
#include <filesystem>
#include <type_traits>

namespace exio {
using stream_handle_t =
    std::invoke_result_t<open_stream_t, std::string_view &, open_flags_t>;
using file_handle_t =
    std::invoke_result_t<open_file_t, std::filesystem::path const &,
                         open_flags_t>;
} // namespace exio
