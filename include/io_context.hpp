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
#include <stdexec/stdexec/execution.hpp>

#ifdef EXIO_LINUX
#include "io_uring/io_uring_context.hpp"
#endif

namespace exio {
#ifdef EXIO_LINUX
using io_context = exio::io_uring_context;
using io_scheduler = exio::io_uring_scheduler;
#endif
} // namespace exio
