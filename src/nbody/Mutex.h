#pragma once

#include <mutex>

template <typename T> class DataMutexLock;

template <typename T> class DataMutex {

public:
  DataMutex() = default;
  DataMutex(const T &data) : m_data(data) {}
  DataMutex(const T &&data) : m_data(std::move(data)) {}
  DataMutex(DataMutex &&other)
      : m_mutex(other.m_mutex), m_data(std::move(other.m_data)) {}
  DataMutex(DataMutex &other) = delete;
  DataMutex operator=(DataMutex &other) = delete;
  DataMutexLock<T> lock();

  ~DataMutex() {
    // Wait until is mutex is unlocked
    m_mutex.lock();
  }

private:
  std::mutex m_mutex;
  T m_data;
};

template <typename T> class DataMutexLock {

public:
  T &m_data;
  DataMutexLock(std::mutex *m, T &t) : m_mutex(m), m_data(t) {
    if (m_mutex) {
      m_mutex->lock();
    }
  }
  DataMutexLock(DataMutexLock &&other)
      : m_mutex(other.m_mutex), m_data(other.m_data) {
    other.m_mutex = nullptr;
  }
  DataMutexLock lock(DataMutex<T> &data) {
    return DataMutexLock(&data.m_mutex, data.m_data);
  }
  ~DataMutexLock() {
    if (m_mutex) {
      m_mutex->unlock();
    }
  }

private:
  std::mutex *m_mutex;
};
