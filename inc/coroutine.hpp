#pragma once

#include <future>
#include <mutex>
#include <thread>

#include "logger.hpp"
#include "utility.hpp"

namespace concurrency {

struct IThreadState {
  virtual ~IThreadState() {}

  virtual void Pause() = 0;
  virtual void Resume() = 0;
};

using IThreadStatePtr = std::shared_ptr<IThreadState>;

struct ThreadState : IThreadState {
  void Pause() override {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_paused = true;
    m_cv.notify_one();
    m_cv.wait(lk, [this] { return !m_paused; });
  }

  void Resume() override {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.wait(lk, [this] { return m_paused; });
    m_paused = false;
    m_cv.notify_one();
  }

private:
  std::mutex m_mutex;
  bool m_paused{false};
  std::condition_variable m_cv;
};

struct LoggableThreadState : IThreadState {
  LoggableThreadState(logger::ThreadSafeLoggerPtr pLogger,
                      IThreadStatePtr pThreadState)
      : m_pLogger{pLogger}, m_pThreadState{pThreadState} {}

  void Pause() {
    m_pLogger->Log("ThreadState::Pause()");
    m_pThreadState->Pause();
  }

  void Resume() {
    m_pLogger->Log("ThreadState::Resume()");
    m_pThreadState->Resume();
  }

private:
  logger::ThreadSafeLoggerPtr m_pLogger;
  IThreadStatePtr m_pThreadState;
};

/*
 * For the @Routine to be executable inside the @Coroutine,
 * it has to support a specific interface, that is it has to receive the
 * @ThreadState as a first argument
 */
template <typename... Ts> struct Coroutine {
public:
  Coroutine() = delete;

  template <typename F, typename... Args>
  Coroutine(F &&routine, Args &&...args)
      : m_routine(std::forward<F>(routine)),
        m_args(std::forward<Args>(args)...), m_pThreadState{
                                                 std::get<0>(m_args)} {}

  Coroutine(const Coroutine &) = default;
  Coroutine &operator=(const Coroutine &) = default;

  Coroutine(Coroutine &&) = delete;
  Coroutine &operator=(Coroutine &&) = delete;

  ~Coroutine() = default;

  template <typename... Args, int... Is>
  void func(std::tuple<Args...> &tup, utility::index<Is...>) {
    m_routine(std::get<Is>(tup)...);
  }

  template <typename... Args> void func(std::tuple<Args...> &tup) {
    func(tup, utility::gen_seq<sizeof...(Args)>{});
  }

  void operator()() { func(m_args); }

  void Resume() { m_pThreadState->Resume(); }

private:
  // The routine which we wrap to make (stop/resume)able
  std::function<void(Ts...)> m_routine;

  // Args given to the function to call it later
  std::tuple<Ts...> m_args;

  // Used to pause and resume the current thread(routine)
  IThreadStatePtr m_pThreadState;
};

template <typename F, typename... Args>
Coroutine<Args...> make_coroutine(F &&f, Args &&...args) {
  return Coroutine<Args...>(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace concurrency
