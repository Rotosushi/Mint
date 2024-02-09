// Copyright (C) 2024 Cade Weinberg
//
// This file is part of Mint.
//
// Mint is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Mint is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Mint.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace mint {
class ThreadPool {
private:
  std::mutex queue_mutex;
  std::condition_variable queue_condition_variable;
  std::queue<std::function<void()>> jobs;
  std::vector<std::thread> pool;
  bool terminate = false;

  void threadLoop() {
    while (true) {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_condition_variable.wait(
            lock, [this] { return !jobs.empty() || terminate; });

        if (terminate) {
          return;
        }

        job = jobs.front();
        jobs.pop();
      }
      job();
    }
  }

public:
  void start() {
    auto num_thread = std::thread::hardware_concurrency();
    for (unsigned i = 0; i < num_thread; ++i) {
      pool.emplace_back(std::thread(&ThreadPool::threadLoop, this));
    }
  }

  void stop() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      terminate = true;
    }
    queue_condition_variable.notify_all();
    for (std::thread &thread : pool) {
      thread.join();
    }
    pool.clear();
  }

  bool busy() {
    bool busy;
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      busy = !jobs.empty();
    }
    return busy;
  }

  void queue(std::function<void()> job) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      jobs.push(job);
    }
    queue_condition_variable.notify_one();
  }
};
} // namespace mint
