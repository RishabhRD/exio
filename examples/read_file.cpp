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
#include <cstdint>
#include <filesystem>
#include <iostream>

int main() {
  using namespace std::chrono_literals;
  exio::io_context ctx;
  auto sch = ctx.get_scheduler();
  auto fd = exio::open(std::filesystem::path("/home/rishabh/myfile"),
                       exio::open_flags::read_only);
  std::array<std::byte, 8> buffer;
  auto task =
      exio::schedule_after(sch, 1s) |
      stdexec::then([] { std::cout << "Hello world!" << std::endl; }) |
      stdexec::let_value([sch, &fd, &buffer] {
        return exio::async_read_some(sch, fd, std::span<std::byte>(buffer));
      }) |
      stdexec::then([&ctx, &buffer](std::uint32_t read_bytes) {
        std::cout << "Read: " << read_bytes << std::endl;
        for (std::uint32_t i{}; i < read_bytes; ++i) {
          std::cout << char(buffer[i]);
        }
        std::cout << std::endl;
        ctx.request_stop();
      });
  stdexec::sync_wait(stdexec::when_all(task, ctx.run()));
}
