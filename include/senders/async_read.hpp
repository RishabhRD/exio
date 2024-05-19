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
#include <cstddef>
#include <span>
#include <stdexec/exec/__detail/__manual_lifetime.hpp>
#include <stdexec/stdexec/execution.hpp>
#include <type_traits>

namespace exio {
namespace __async_read {
using namespace stdexec::tags;

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_operation_state;

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_receiver {
  async_read_operation_state<Scheduler, Handle, Receiver> *op;

  using receiver_concept = stdexec::receiver_t;

  void set_value(std::size_t num_bytes) noexcept;
  void set_error(std::exception_ptr e) noexcept;
  void set_stopped() noexcept;
  auto get_env() const noexcept;
};

template <typename Scheduler, typename Handle, typename Receiver>
struct async_read_operation_state {
  using outer_receiver = async_read_receiver<Scheduler, Handle, Receiver>;
  using inner_sender =
      std::invoke_result_t<exio::async_read_some_t, Scheduler const &, Handle &,
                           std::span<std::byte>>;
  using child_op_state_t =
      stdexec::connect_result_t<inner_sender, outer_receiver>;
  Scheduler sch;
  Handle &handle;
  std::span<std::byte> buffer;
  Receiver rcvr;
  exec::__manual_lifetime<child_op_state_t> child_op_state;

  async_read_operation_state(async_read_operation_state const &) = delete;
  async_read_operation_state(async_read_operation_state &&) = delete;
  auto operator=(async_read_operation_state) = delete;

  async_read_operation_state(Scheduler sch_, Handle &handle_,
                             std::span<std::byte> buffer_, Receiver &&rcvr_)
      : sch(sch_), handle(handle_), buffer(buffer_),
        rcvr(static_cast<Receiver &&>(rcvr_)) {
    connect();
  }

  void start() & noexcept { start_child(); }

  auto connect() {
    child_op_state.__construct_with([this] {
      return stdexec::connect(exio::async_read_some(sch, handle, buffer),
                              outer_receiver{.op = this});
    });
  }

  auto reconnect() {
    child_op_state.__destroy();
    connect();
  }

  auto start_child() noexcept { stdexec::start(child_op_state.__get()); }

  auto complete(std::size_t num_bytes_read) {
    if (num_bytes_read < buffer.size()) {
      using diff_type = std::span<std::byte>::difference_type;
      buffer = std::span{std::begin(buffer) + diff_type(num_bytes_read),
                         std::end(buffer)};
      reconnect();
      start_child();
    } else {
      stdexec::set_value(static_cast<Receiver &&>(rcvr));
    }
  }

  template <typename... Args> auto complete_error(Args &&...args) {
    stdexec::set_error(static_cast<Receiver &&>(rcvr),
                       std::forward<Args>(args)...);
  }

  auto complete_stopped() {
    stdexec::set_stopped(static_cast<Receiver &&>(rcvr));
  }

  ~async_read_operation_state() noexcept { child_op_state.__destroy(); }
};

template <typename Scheduler, typename Handle, typename Receiver>
void async_read_receiver<Scheduler, Handle, Receiver>::set_value(
    std::size_t num_bytes_read) noexcept {
  op->complete(num_bytes_read);
}
template <typename Scheduler, typename Handle, typename Receiver>
void async_read_receiver<Scheduler, Handle, Receiver>::set_error(
    std::exception_ptr ptr) noexcept {
  op->complete_error(ptr);
}
template <typename Scheduler, typename Handle, typename Receiver>
void async_read_receiver<Scheduler, Handle, Receiver>::set_stopped() noexcept {
  op->complete_stopped();
}
template <typename Scheduler, typename Handle, typename Receiver>
auto async_read_receiver<Scheduler, Handle, Receiver>::get_env()
    const noexcept {
  return stdexec::empty_env{};
}

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

  auto get_env() const noexcept -> env_t { return env; }

  template <class Env>
  auto get_completion_signatures(Env) const noexcept -> completion_sigs {
    return {};
  }

  template <stdexec::receiver_of<completion_sigs> Receiver>
  auto connect(Receiver &&rcvr) const {
    return async_read_operation_state<Scheduler, Handle, Receiver>(
        env.sch, handle, buffer, static_cast<Receiver &&>(rcvr));
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