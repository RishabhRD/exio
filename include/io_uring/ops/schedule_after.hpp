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
template <class _ReceiverId> struct __schedule_after_operation {
  using _Receiver = stdexec::__t<_ReceiverId>;

  class __impl : public __stoppable_op_base<_Receiver> {
#ifdef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
    struct __kernel_timespec {
      __s64 __tv_sec;
      __s64 __tv_nsec;
    };

    __kernel_timespec __duration_;

    static constexpr auto
    __duration_to_timespec(std::chrono::nanoseconds dur) noexcept
        -> __kernel_timespec {
      auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
      dur -= secs;
      secs = std::max(secs, std::chrono::seconds{0});
      dur = std::clamp(dur, std::chrono::nanoseconds{0},
                       std::chrono::nanoseconds{999'999'999});
      return __kernel_timespec{secs.count(), dur.count()};
    }
#else
    safe_file_descriptor __timerfd_;
    ::itimerspec __duration_;
    std::uint64_t __n_expirations_{0};
    ::iovec __iov_{&__n_expirations_, sizeof(__n_expirations_)};

    static constexpr ::itimerspec
    __duration_to_timespec(std::chrono::nanoseconds __nsec) noexcept {
      ::itimerspec __timerspec{};
      ::clock_gettime(CLOCK_REALTIME, &__timerspec.it_value);
      __nsec = std::chrono::nanoseconds{__timerspec.it_value.tv_nsec} + __nsec;
      auto __sec = std::chrono::duration_cast<std::chrono::seconds>(__nsec);
      __nsec -= __sec;
      __nsec = std::clamp(__nsec, std::chrono::nanoseconds{0},
                          std::chrono::nanoseconds{999'999'999});
      __timerspec.it_value.tv_sec += __sec.count();
      __timerspec.it_value.tv_nsec = __nsec.count();
      STDEXEC_ASSERT(0 <= __timerspec.it_value.tv_nsec &&
                     __timerspec.it_value.tv_nsec < 1'000'000'000);
      return __timerspec;
    }
#endif

  public:
    static constexpr auto ready() noexcept -> std::false_type { return {}; }

#ifndef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
    void submit_stop(::io_uring_sqe &__sqe) noexcept {
      __duration_.it_value.tv_sec = 1;
      __duration_.it_value.tv_nsec = 0;
      ::timerfd_settime(__timerfd_, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
                        &__duration_, nullptr);
      __sqe = ::io_uring_sqe{.opcode = IORING_OP_NOP};
    }
#endif

    void submit(::io_uring_sqe &__sqe) noexcept {
#ifdef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
      ::io_uring_sqe __sqe_{};
      __sqe_.opcode = IORING_OP_TIMEOUT;
      __sqe_.addr = bit_cast<__u64>(&__duration_);
      __sqe_.len = 1;
      __sqe = __sqe_;
#else
      ::io_uring_sqe __sqe_{};
      __sqe_.opcode = IORING_OP_READV;
      __sqe_.fd = __timerfd_;
      __sqe_.addr = bit_cast<__u64>(&__iov_);
      __sqe_.len = 1;
      __sqe = __sqe_;
#endif
    }

    void complete(const ::io_uring_cqe &__cqe) noexcept {
#ifdef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
      if (__cqe.res == -ETIME || __cqe.res == 0) {
#else
      if (__cqe.res == sizeof(std::uint64_t)) {
#endif
        stdexec::set_value(static_cast<_Receiver &&>(this->__receiver_));
      } else {
        STDEXEC_ASSERT(__cqe.res < 0);
        stdexec::set_error(static_cast<_Receiver &&>(this->__receiver_),
                           std::make_exception_ptr(std::system_error(
                               -__cqe.res, std::system_category())));
      }
    }

    __impl(context_t &__context, std::chrono::nanoseconds __duration,
           _Receiver &&__receiver)
        : __stoppable_op_base<_Receiver>{__context,
                                         static_cast<_Receiver &&>(__receiver)}
#ifdef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
          ,
          __duration_{__duration_to_timespec(__duration)}
#else
          ,
          __timerfd_{::timerfd_create(CLOCK_REALTIME, 0)},
          __duration_{__duration_to_timespec(__duration)}
#endif
    {
#ifndef STDEXEC_HAS_IO_URING_ASYNC_CANCELLATION
      int __rc = ::timerfd_settime(__timerfd_,
                                   TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
                                   &__duration_, nullptr);
      __throw_error_code_if(__rc < 0, errno);
#endif
    }
  };

  using __t = __stoppable_task_facade_t<__impl>;
};

template <typename Scheduler> class __schedule_after_sender {
public:
  using sender_concept = stdexec::sender_t;
  using __id = __schedule_after_sender;
  using __t = __schedule_after_sender;

  using env_t = io_uring_env_t<Scheduler>;
  env_t __env_;
  std::chrono::nanoseconds __duration_;

private:
  auto get_env() const noexcept -> env_t { return __env_; }

  using __completion_sigs =
      stdexec::completion_signatures<stdexec::set_value_t(),
                                     stdexec::set_error_t(std::exception_ptr),
                                     stdexec::set_stopped_t()>;

  // TODO converting to member function doesn't work for this
  template <class _Env>
  STDEXEC_MEMFN_DECL(auto get_completion_signatures)
  (this const __schedule_after_sender &, _Env) noexcept -> __completion_sigs {
    return {};
  }

  // TODO converting to member function doesn't work for this
  template <stdexec::receiver_of<__completion_sigs> _Receiver>
  STDEXEC_MEMFN_DECL(auto connect)
  (this const __schedule_after_sender &__sender, _Receiver &&__receiver)
      -> stdexec::__t<__schedule_after_operation<stdexec::__id<_Receiver>>> {
    return stdexec::__t<__schedule_after_operation<stdexec::__id<_Receiver>>>(
        std::in_place, *__sender.__env_.ctx, __sender.__duration_,
        static_cast<_Receiver &&>(__receiver));
  }
};

} // namespace __io_uring
} // namespace exio
