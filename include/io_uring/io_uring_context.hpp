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
#include "io_uring/ops/async_write_some.hpp"
#include "io_uring/ops/schedule.hpp"
#include "io_uring/ops/schedule_after.hpp"
#include "posix/file/file_handle.hpp"
#include <chrono>
#include <span>

namespace exio {
namespace __io_uring {

class scheduler_t {
public:
  context_t *__context_;

  friend auto operator==(const scheduler_t &__lhs, const scheduler_t &__rhs)
      -> bool = default;

  auto schedule() const -> __schedule_sender<scheduler_t> {
    return __schedule_sender<scheduler_t>{.env = {__context_}};
  }

  auto now() const noexcept { return std::chrono::steady_clock::now(); }

  auto schedule_after(std::chrono::nanoseconds __duration) const {
    return __schedule_after_sender<scheduler_t>{.__env_ = {__context_},
                                                .__duration_ = __duration};
  }

  template <class _Clock, class _Duration>
  auto schedule_at(
      std::chrono::time_point<_Clock, _Duration> const &__time_point) const {
    auto __duration = __time_point - _Clock::now();
    return __schedule_after_sender<scheduler_t>{.__env_ = {__context_},
                                                .__duration_ = __duration};
  }

  template <bool is_stream, bool is_file, bool is_socket>
    requires(is_stream == true)
  inline auto async_read_some_at(
      exio::posix::file_handle<is_stream, is_file, is_socket> &file,
      std::size_t offset, std::span<std::byte> buffer) const {
    return async_read_some_sender<scheduler_t>{
        .fd = file.fd, .buffer = buffer, .offset = offset, .env = {__context_}};
  }

  template <bool is_stream, bool is_file, bool is_socket>
    requires(is_stream == true)
  inline auto async_write_some_at(
      exio::posix::file_handle<is_stream, is_file, is_socket> &file,
      std::size_t offset, std::span<std::byte const> buffer) const {
    return async_write_some_sender<scheduler_t>{
        .fd = file.fd, .buffer = buffer, .offset = offset, .env = {__context_}};
  }

  template <bool is_stream, bool is_file, bool is_socket>
    requires(is_stream == true)
  inline auto
  async_read_some(exio::posix::file_handle<is_stream, is_file, is_socket> &file,
                  std::span<std::byte> buffer) const {
    return async_read_some_at(file, 0, buffer);
  }

  template <bool is_stream, bool is_file, bool is_socket>
    requires(is_stream == true)
  inline auto async_write_some(
      exio::posix::file_handle<is_stream, is_file, is_socket> &file,
      std::span<std::byte const> buffer) const {
    return async_write_some_at(file, 0, buffer);
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
