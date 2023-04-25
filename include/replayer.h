#ifndef UDES_REPLAY_REPLAYER_H
#define UDES_REPLAY_REPLAYER_H

#include <iostream>
#include <functional>
#include <chrono>
#include <thread>
#include <memory>

#include "buffer.h"

template<typename K, typename V, std::size_t N>
class replayer {
public:
  using value_type = std::pair<K, V>;

private:
  std::array<buffer<value_type, N>, 3> buffers{};
  std::atomic<std::size_t> consumer_idx{0};
  std::atomic<std::size_t> producer_idx{2};
  std::atomic<bool> running = true;
  std::shared_ptr<storage<std::pair<K, V>>> db;

public:
  explicit replayer(std::shared_ptr<storage<std::pair<K, V>>> db) : db{db} {}
  ~replayer() = default;

  void produce() {
    while (is_running()) {
      if (buffers[producer_idx.load()].is_empty()) {
        if (!db.get()->read(&buffers[producer_idx.load()]))
          return;
        buffers[producer_idx.load()].resize(N);
      }
    }
  }

  template<typename Func>
  void consume(Func consumer) {
    while (!buffers[consumer_idx.load()].is_empty()) {
      for (auto [delay, val]: buffers[consumer_idx.load()]) {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
        consumer(val);
      }
      buffers[consumer_idx.load()].resize(0);
      swap_buffers();
    }
    running.store(false);
  }

  void preload() {
    for (std::size_t i = 0; i != 3; ++i) {
      db.get()->read(&buffers[i]);
    }
  }

private:
  void swap_buffers() {
    consumer_idx.store((consumer_idx.load() + 1) % 3);
    producer_idx.store((producer_idx.load() + 1) % 3);
  }

  bool is_running() { return running.load(); }
};

#endif // UDES_REPLAY_REPLAYER_H