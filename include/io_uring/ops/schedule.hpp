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
#include <stdexec/stdexec/execution.hpp>

namespace exio {
namespace __io_uring {
template <class _ReceiverId> struct __schedule_operation {
  using _Receiver = stdexec::__t<_ReceiverId>;

  struct __impl {
    context_t &__context_;
    STDEXEC_ATTRIBUTE((no_unique_address))
    _Receiver __receiver_;

    __impl(context_t &__context, _Receiver &&__receiver)
        : __context_{__context},
          __receiver_{static_cast<_Receiver &&>(__receiver)} {}

    [[nodiscard]] auto context() const noexcept -> context_t & {
      return __context_;
    }

    static constexpr auto ready() noexcept -> std::true_type { return {}; }

    static constexpr void submit(::io_uring_sqe &) noexcept {}

    void complete(const ::io_uring_cqe &__cqe) noexcept {
      auto token = stdexec::get_stop_token(stdexec::get_env(__receiver_));
      if (__cqe.res == -ECANCELED || __context_.stop_requested() ||
          token.stop_requested()) {
        stdexec::set_stopped(static_cast<_Receiver &&>(__receiver_));
      } else {

        stdexec::set_value(static_cast<_Receiver &&>(__receiver_));
      }
    }
  };

  using __t = __io_task_facade<__impl>;
};

template <typename Scheduler> class __schedule_sender {
public:
  using env_t = io_uring_env_t<Scheduler>;
  env_t env;

  using sender_concept = stdexec::sender_t;
  using __id = __schedule_sender;
  using __t = __schedule_sender;

private:
  using __completion_sigs =
      stdexec::completion_signatures<stdexec::set_value_t(),
                                     stdexec::set_stopped_t()>;

  // TODO doens't work with member function
  STDEXEC_MEMFN_DECL(auto get_env)
  (this const __schedule_sender &__sender) noexcept -> env_t {
    return __sender.env;
  }

  template <class _Env>
  auto get_completion_signatures(_Env) const noexcept -> __completion_sigs {
    return {};
  }

  template <stdexec::receiver_of<__completion_sigs> _Receiver>
  auto connect(_Receiver &&__receiver) const
      -> stdexec::__t<__schedule_operation<stdexec::__id<_Receiver>>> {
    return stdexec::__t<__schedule_operation<stdexec::__id<_Receiver>>>(
        std::in_place, *(env.ctx), static_cast<_Receiver &&>(__receiver));
  }
};
} // namespace __io_uring
} // namespace exio
