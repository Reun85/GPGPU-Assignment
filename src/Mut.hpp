#pragma once
#include <mutex>

namespace DataMutex {
template <typename Ty>
class MutexData {
  MutexData(std::mutex &m, Ty &val) : mutex(m), data(val) { mutex.lock(); }

  Ty &operator*() const { return data; }

  ~MutexData() { mutex.unlock(); }

 protected:
  Ty &data;
  std::mutex &mutex;
};
template <typename Ty>
class MutexProtected {
  MutexProtected(const Ty &data) : value(data) {}
  MutexProtected(const Ty &&data) : value(std::move(data)) {}
  template <typename... Args>
  MutexProtected(Args... args) : value(args...) {}
  MutexProtected(Ty val) : value(val) {}

  MutexProtected(MutexProtected &other) = delete;
  MutexProtected operator=(MutexProtected &other) = delete;
  MutexProtected() {}
  ~MutexProtected() { mutex.lock(); }

 protected:
  Ty value;
  std::mutex mutex;
};

}  // namespace DataMutex
