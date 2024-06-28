#pragma once
#include <mutex>

template <typename Ty>
class MutexData {
 public:
  MutexData(std::mutex &m, Ty &val) : mutex(m), data(val) { mutex.lock(); }

  Ty &operator*() const { return data; }
  Ty &operator->() const { return data; }

  ~MutexData() { mutex.unlock(); }

 protected:
  Ty &data;
  std::mutex &mutex;
};
template <typename Ty>
class MutexProtected {
 public:
  MutexProtected(const Ty &data) : value(data) {}
  MutexProtected(const Ty &&data) : value(std::move(data)) {}
  template <typename... Args>
  MutexProtected(Args... args) : value(args...) {}

  MutexProtected(MutexProtected &other) = delete;
  MutexProtected operator=(MutexProtected &other) = delete;
  MutexProtected() {}
  MutexData<Ty> lock() { return MutexData<Ty>(mutex, value); }
  ~MutexProtected() { mutex.lock(); }

 protected:
  Ty value;
  std::mutex mutex;
};