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

#include "exio.hpp"
#include "io_context.hpp"
#include <chrono>
#include <concepts>
#include <iostream>
#include <stdexec/exec/task.hpp>
#include <stdexec/exec/when_any.hpp>

using namespace std::chrono_literals;

auto read_stream(exio::io_scheduler sch, std::string_view path)
    -> exec::task<void> {
  std::cout << "Waiting for a sec" << std::endl;
  co_await exio::schedule_after(sch, 1s);
  auto file = exio::open_stream(path, exio::open_flags::read_only);
  std::cout << "Starting to read file" << std::endl;
  std::array<std::byte, 8> buffer;
  auto sndr = exio::async_read(sch, file, buffer);
  using sndr_t = decltype(sndr);
  co_await sndr;
  std::cout << "Content: ";
  for (auto c : buffer) {
    std::cout << char(c);
  }
  std::cout << std::endl;
}

auto say_bye_after(exio::io_context &ctx, int sec) -> exec::task<void> {
  co_await exio::schedule_after(ctx.get_scheduler(), std::chrono::seconds(sec));
  std::cout << "Couldn't read file in time, Byee!!" << std::endl;
}

int main() {
  exio::io_context ctx;
  auto sch = ctx.get_scheduler();
  std::string_view path{"/dev/random"};
  std::jthread io_thread{[&] { ctx.run_until_stopped(); }};
  stdexec::sync_wait(
      exec::when_any(read_stream(sch, path), say_bye_after(ctx, 4)) |
      stdexec::then([&ctx] { ctx.request_stop(); }));
}
