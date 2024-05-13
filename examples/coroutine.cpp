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
#include <filesystem>
#include <iostream>
#include <stdexec/exec/task.hpp>
#include <stdexec/exec/timed_scheduler.hpp>

using namespace std::chrono_literals;

auto print_file_details(exio::io_scheduler sch,
                        std::filesystem::path const &path) -> exec::task<void> {
  std::cout << "Waiting for a sec" << std::endl;
  co_await exec::schedule_after(sch, 1s);
  std::cout << "Starting to read file" << std::endl;
  auto file = exio::open(path, exio::open_flags::read_only);
  // TODO: async_read_some can't be used as coroutine
  //
  // std::array<std::byte, 8> buffer;
  // auto num_bytes = co_await exio::async_read_some(sch, file, buffer);
  // std::cout << "Content: ";
  // for (std::size_t i{}; i < num_bytes; ++i) {
  //   std::cout << char(buffer[i]);
  // }
  // std::cout << std::endl;
}

auto say_bye_after(exio::io_scheduler sch, int sec) -> exec::task<void> {
  co_await exec::schedule_after(sch, std::chrono::seconds(sec));
  std::cout << "Bye!" << std::endl;
}

int main() {
  exio::io_context ctx;
  auto sch = ctx.get_scheduler();
  std::filesystem::path path{"/home/rishabh/myfile"};
  std::jthread io_thread{[&] { ctx.run_until_empty(); }};
  stdexec::sync_wait(
      stdexec::when_all(print_file_details(sch, path), say_bye_after(sch, 5)));
}