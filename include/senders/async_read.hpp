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

#include "senders/__details/env.hpp"
#include "senders/async_read_some.hpp"
#include <span>
#include <stdexec/stdexec/execution.hpp>
#include <type_traits>
namespace exio {
namespace __async_read {
using namespace stdexec::tags;

template <typename Scheduler, typename Handle>
using async_read_some_sender_type =
    std::invoke_result_t<exio::async_read_some_t, Scheduler, Handle &,
                         std::span<std::byte>>;

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_operation_state;

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_receiver {
  async_read_operation_state<Scheduler, Handle, Receiver> *op;

  using receiver_concept = stdexec::receiver_t;

  STDEXEC_MEMFN_DECL(auto set_value)
  (this async_read_receiver const &self, std::size_t bytes_read) {
    self.op->complete(bytes_read);
  }

  STDEXEC_MEMFN_DECL(auto get_env)
  (this async_read_receiver const &self) noexcept {
    return stdexec::get_env(self.op->rcvr);
  }
};

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_operation_state {
  using higher_order_receiver =
      async_read_receiver<Scheduler, Handle, Receiver>;
  using child_op_state_t =
      stdexec::connect_result_t<async_read_some_sender_type<Scheduler, Handle>,
                                higher_order_receiver>;
  Scheduler sch;
  Handle &handle;
  std::span<std::byte> buffer;
  Receiver rcvr;
  child_op_state_t child_op_state;

  async_read_operation_state(async_read_operation_state const &) = delete;
  async_read_operation_state(async_read_operation_state &&) = delete;
  auto operator=(async_read_operation_state) = delete;

  async_read_operation_state(Scheduler sch_, Handle &handle_,
                             std::span<std::byte> buffer_, Receiver &&rcvr_)
      : sch(sch_), handle(handle_), buffer(buffer_),
        rcvr(static_cast<Receiver &&>(rcvr_)), child_op_state(connect()) {}

  STDEXEC_MEMFN_DECL(auto start)
  (this async_read_operation_state &self) noexcept {
    stdexec::start(self.child_op_state);
  }

  auto connect() const {
    auto sndr = exio::async_read_some(sch, handle, buffer);
    return stdexec::connect(
        sndr, async_read_receiver<Scheduler, Handle, Receiver>{.op = this});
  }

  auto complete(std::size_t num_bytes_read) {
    if (num_bytes_read < buffer.size()) {
      buffer = std::span{std::begin(buffer) + num_bytes_read, std::end(buffer)};
      child_op_state = connect();
    } else {
      stdexec::set_value(static_cast<Receiver &&>(rcvr));
    }
  }
};

template <typename Scheduler, typename Handle> struct async_read_sender {
  using env_t = __env_details::env_t<Scheduler>;

  Handle &handle;
  std::span<std::byte> buffer;
  env_t env;

  using sender_concept = stdexec::sender_t;

  using completion_sigs =
      stdexec::completion_signatures<stdexec::set_value_t(),
                                     stdexec::set_error_t(std::exception_ptr),
                                     stdexec::set_stopped_t()>;

  STDEXEC_MEMFN_DECL(auto get_env)
  (this async_read_sender const &sender) noexcept -> env_t {
    return sender.env;
  }

  template <class Env>
  STDEXEC_MEMFN_DECL(auto get_completion_signatures)
  (this async_read_sender const &, Env) noexcept -> completion_sigs {
    return {};
  }

  template <stdexec::receiver_of<completion_sigs> Receiver>
  STDEXEC_MEMFN_DECL(auto connect)
  (this async_read_sender const &self, Receiver &&rcvr) {
    return async_read_operation_state<Scheduler, Handle, Receiver>(
        self.env.sch, self.handle, self.buffer, static_cast<Receiver &&>(rcvr));
  }
};
} // namespace __async_read

struct async_read_t {
  template <stream_io_scheduler Scheduler, typename handle_t>
    requires(std::same_as<handle_t, stream_handle_t>)
  auto operator()(Scheduler const &sch, handle_t &handle,
                  std::span<std::byte> buffer) const {
    if constexpr (requires {
                    {
                      sch.async_read(handle, buffer)
                    } -> stdexec::sender_of<stdexec::set_value_t(void)>;
                  }) {
      return sch.async_read(handle, buffer);
    } else {
      return __async_read::async_read_sender<Scheduler, handle_t>{
          .handle = handle, .buffer = buffer, .env = {.sch = sch}};
    }
  }
};
constexpr async_read_t async_read{};
} // namespace exio
