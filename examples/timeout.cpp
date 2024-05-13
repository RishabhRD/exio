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
#include <iostream>
#include <stdexec/exec/timed_scheduler.hpp>
int main() {
  using namespace std::chrono_literals;
  exio::io_context ctx;
  auto sch = ctx.get_scheduler();
  auto task = exec::schedule_after(sch, 1s) //
              | stdexec::then([&ctx] {
                  std::cout << "Hello world!" << std::endl;
                  ctx.request_stop();
                }); //
  stdexec::sync_wait(stdexec::when_all(task, ctx.run()));
}
