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

#include "io_uring/env.hpp"
#include "io_uring/io_uring_context_base.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexec/exec/linux/safe_file_descriptor.hpp>
#include <stdexec/stdexec/execution.hpp>
#include <utility>

namespace exio {
namespace __io_uring {
using namespace stdexec::tags;

template <typename ReceiverId, std::size_t Extent>
struct async_write_some_operation {
  using Receiver = stdexec::__t<ReceiverId>;
  struct impl : public __stoppable_op_base<Receiver> {
    int fd;
    std::span<std::byte const, Extent> buffer;
    std::size_t offset;

    impl(context_t &ctx_, int fd_, std::span<std::byte const, Extent> buffer_,
         std::size_t offset_, Receiver &&receiver_)
        : __stoppable_op_base<Receiver>{ctx_,
                                        static_cast<Receiver &&>(receiver_)},
          fd{fd_}, buffer{buffer_}, offset{offset_} {}

    static constexpr auto ready() noexcept -> std::false_type { return {}; }

    void submit(::io_uring_sqe &sqe) noexcept {
      io_uring_sqe new_sqe{};
      new_sqe.opcode = IORING_OP_WRITE;
      new_sqe.fd = fd;
      new_sqe.addr = std::uint64_t(buffer.data());
      new_sqe.len = std::uint32_t(buffer.size());
      new_sqe.off = offset;
      sqe = new_sqe;
    }

    void complete(::io_uring_cqe const &cqe) noexcept {
      if (cqe.res < 0) {
        stdexec::set_error(static_cast<Receiver &&>(this->__receiver_),
                           std::make_exception_ptr(std::system_error(
                               -cqe.res, std::system_category())));
      } else {
        stdexec::set_value(static_cast<Receiver &&>(this->__receiver_),
                           static_cast<std::size_t>(cqe.res));
      }
    }
  };

  using __t = __stoppable_task_facade_t<impl>;
};

template <typename Scheduler, std::size_t Extent>
struct async_write_some_sender {
  int fd;
  std::span<std::byte const, Extent> buffer;
  using env_t = io_uring_env_t<Scheduler>;
  std::size_t offset;
  env_t env;

  using sender_concept = stdexec::sender_t;
  using __id = async_write_some_sender;
  using __t = async_write_some_sender;

  using completion_sigs =
      stdexec::completion_signatures<stdexec::set_value_t(std::size_t),
                                     stdexec::set_error_t(std::exception_ptr),
                                     stdexec::set_stopped_t()>;

  auto get_env() const noexcept -> env_t { return env; }

  template <class Env>
  auto get_completion_signatures(Env) const noexcept -> completion_sigs {
    return {};
  }

  template <stdexec::receiver_of<completion_sigs> Receiver>
  auto connect(Receiver &&receiver) const -> stdexec::__t<
      async_write_some_operation<stdexec::__id<Receiver>, Extent>> {
    return stdexec::__t<
        async_write_some_operation<stdexec::__id<Receiver>, Extent>>(
        std::in_place, *(env.ctx), fd, buffer, offset,
        static_cast<Receiver &&>(receiver));
  }
};

} // namespace __io_uring

} // namespace exio
