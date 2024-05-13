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

#include "io_uring/io_uring_context_base.hpp"
#include "io_uring/ops/async_read_some.hpp"
#include "io_uring/ops/schedule.hpp"
#include "io_uring/ops/schedule_after.hpp"
#include "unix/file_handle.hpp"

namespace exio {
namespace __io_uring {

class scheduler_t {
public:
  context_t *__context_;

  friend auto operator==(const scheduler_t &__lhs, const scheduler_t &__rhs)
      -> bool = default;

  STDEXEC_MEMFN_FRIEND(schedule);

  STDEXEC_MEMFN_DECL(auto schedule)
  (this const scheduler_t &__sched)->__schedule_sender<scheduler_t> {
    return __schedule_sender<scheduler_t>{.env = {__sched.__context_}};
  }

  friend auto tag_invoke(exec::now_t, const scheduler_t &) noexcept
      -> std::chrono::time_point<std::chrono::steady_clock> {
    return std::chrono::steady_clock::now();
  }

  friend auto tag_invoke(exec::schedule_after_t, const scheduler_t &__sched,
                         std::chrono::nanoseconds __duration) {
    return __schedule_after_sender<scheduler_t>{.__env_ = {__sched.__context_},
                                                .__duration_ = __duration};
  }

  template <class _Clock, class _Duration>
  friend auto
  tag_invoke(exec::schedule_at_t, const scheduler_t &__sched,
             const std::chrono::time_point<_Clock, _Duration> &__time_point) {
    auto __duration = __time_point - _Clock::now();
    return __schedule_after_sender<scheduler_t>{.__env_ = {__sched.__context_},
                                                .__duration_ = __duration};
  }

  template <open_mode mode, std::size_t Extent>
    requires((mode == open_mode::READ_ONLY) || (mode == open_mode::READ_WRITE))
  inline auto async_read_some(exio::file_handle<mode> &file,
                              std::span<std::byte, Extent> buffer) const {
    return async_read_some_sender<scheduler_t, Extent>{
        .fd = file.fd, .buffer = buffer, .env = {__context_}};
  }
};

inline auto context_t::get_scheduler() noexcept -> scheduler_t {
  return scheduler_t{this};
}

inline auto get_context(scheduler_t sch) noexcept -> context_t & {
  return *(sch.__context_);
}
} // namespace __io_uring
using __io_uring::until;
using io_uring_context = __io_uring::context_t;
using io_uring_scheduler = __io_uring::scheduler_t;
} // namespace exio
