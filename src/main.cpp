#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "coroutine.hpp"
#include "logger.hpp"
#include "utility.hpp"

using namespace std::chrono_literals;

void Printer(concurrency::IThreadStatePtr threadPause,
             logger::ThreadSafeLoggerPtr logger, int a, int b) {
  logger->Log("Hello from experimental coroutine!!!");
  logger->Log("Now I will pause myself, huh!");
  threadPause->Pause();
  int c = a + b;
  logger->Log("YOU RESUMED ME! a+b=" + std::to_string(c));
  threadPause->Pause();
  logger->Log("AGAIN! YOU RESUMED ME!");
};

int main() {
  auto logger = std::make_shared<logger::ThreadSafeLogger>();

  auto coro1 = concurrency::make_coroutine(
      Printer,
      std::make_shared<concurrency::LoggableThreadState>(
          logger, std::make_shared<concurrency::ThreadState>()),
      logger, 4, 5);

  auto coro2 = concurrency::make_coroutine(
      Printer,
      std::make_shared<concurrency::LoggableThreadState>(
          logger, std::make_shared<concurrency::ThreadState>()),
      logger, -1, 5);

  std::thread worker(std::ref(coro1));
  std::thread slave(std::ref(coro2));

  logger->Log("Main before coro1 resume");
  coro1.Resume();
  logger->Log("Main after coro1 resume");
  logger->Log("Main before coro2 resume");
  coro2.Resume();
  logger->Log("Main after second resume");

  coro2.Resume();
  coro1.Resume();

  slave.join();
  worker.join();

  return 0;
}
