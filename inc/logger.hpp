#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace logger {
struct ThreadSafeLogger {
  void Log(const std::string &msg) {
    std::lock_guard<std::mutex> lg(m_mutex);
    std::cout << "[thread_id:" << std::this_thread::get_id() << "]: " << msg
              << std::endl;
  }

private:
  std::mutex m_mutex;
};

using ThreadSafeLoggerPtr = std::shared_ptr<ThreadSafeLogger>;
} // namespace logger
