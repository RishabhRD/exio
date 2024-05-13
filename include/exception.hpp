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

#include <utility>
namespace exio {
namespace _exceptions {
struct throw_t {
  template <typename Exception>
  inline void operator()([[maybe_unused]] Exception &&ex) const {
#ifdef EXIO_NO_EXCEPTIONS
    std::terminate();
#else
    throw std::forward<Exception>(ex);
#endif
  }
};
} // namespace _exceptions
inline constexpr _exceptions::throw_t throw_{};
} // namespace exio
