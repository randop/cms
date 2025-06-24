#pragma once

#ifndef CMS_KEYVALUECACHE_HPP
#define CMS_KEYVALUECACHE_HPP

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace services {

class KeyValueCache {
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using Duration = Clock::duration;

  struct KeyValue {
    std::string key;
    std::string value;
    TimePoint expiry;
    bool isExpired() const { return Clock::now() >= expiry; }
  };

  explicit KeyValueCache(size_t capacity)
      : buffer(capacity), capacity(capacity) {}

  // Set a key-value pair with TTL (seconds); returns false if buffer is full.
  bool set(const std::string &key, const std::string &value,
           int64_t ttlSeconds) {
    std::lock_guard<std::mutex> lock(mutex);
    if (sizeCount == capacity) {
      // Try to evict expired items.
      evictExpired();
      if (sizeCount == capacity) {
        return false;
      }
    }

    // Update existing key if present.
    for (size_t i = 0; i < sizeCount; ++i) {
      size_t index = (head + i) % capacity;
      if (buffer[index].key == key && !buffer[index].isExpired()) {
        buffer[index].value = value;
        buffer[index].expiry =
            Clock::now() + Duration(std::chrono::seconds(ttlSeconds));
        return true;
      }
    }

    // Add new key-value pair.
    size_t index = (head + sizeCount) % capacity;
    buffer[index] = KeyValue{
        key, value, Clock::now() + Duration(std::chrono::seconds(ttlSeconds))};
    ++sizeCount;
    return true;
  }

  // Get a value by key; returns empty optional if not found or expired.
  std::optional<std::string> get(const std::string &key) {
    std::lock_guard<std::mutex> lock(mutex);
    for (size_t i = 0; i < sizeCount; ++i) {
      size_t index = (head + i) % capacity;
      if (buffer[index].key == key) {
        if (buffer[index].isExpired()) {
          // Remove expired item.
          removeAtIndex(i);
          return std::nullopt;
        }
        return buffer[index].value;
      }
    }
    return std::nullopt;
  }

  // Remove a key; returns true if key was found and removed.
  bool remove(const std::string &key) {
    std::lock_guard<std::mutex> lock(mutex);
    for (size_t i = 0; i < sizeCount; ++i) {
      size_t index = (head + i) % capacity;
      if (buffer[index].key == key) {
        removeAtIndex(i);
        return true;
      }
    }
    return false;
  }

  // Get current size (excluding expired items).
  size_t size() {
    std::lock_guard<std::mutex> lock(mutex);
    evictExpired();
    return sizeCount;
  }

  bool empty() const { return sizeCount == 0; }

private:
  void evictExpired() {
    for (size_t i = 0; i < sizeCount; ++i) {
      size_t index = (head + i) % capacity;
      if (buffer[index].isExpired()) {
        removeAtIndex(i);
        --i; // Re-check the same index after removal.
      }
    }
  }

  void removeAtIndex(size_t pos) {
    if (pos >= sizeCount)
      return;

    size_t index = (head + pos) % capacity;
    if (pos == 0) {
      // Remove from head.
      buffer[index] = KeyValue{};
      head = (head + 1) % capacity;
      --sizeCount;
    } else {
      // Shift elements left.
      for (size_t i = pos; i < sizeCount - 1; ++i) {
        size_t curr = (head + i) % capacity;
        size_t next = (head + i + 1) % capacity;
        buffer[curr] = std::move(buffer[next]);
      }
      buffer[(head + sizeCount - 1) % capacity] = KeyValue{};
      --sizeCount;
    }
  }

  std::vector<KeyValue> buffer;
  size_t capacity;
  size_t head = 0;
  size_t sizeCount = 0;
  mutable std::mutex mutex;
};

} // namespace services

#endif // CMS_KEYVALUECACHE_HPP