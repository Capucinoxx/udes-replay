#ifndef UDES_REPLAY_RECORDER_H
#define UDES_REPLAY_RECORDER_H

#include <iostream>
#include <functional>
#include <memory>

#include "buffer.h"
#include "storage.h"

template<typename K, typename V, std::size_t N>
class recorder {
public:
  using value_type = std::pair<K, V>;

private:
  std::array<buffer<value_type, N>, 2> buffers{};
  std::atomic<std::size_t> consumer_idx{0};
  std::atomic<std::size_t> producer_idx{1};
  std::atomic<bool> running = false;
  std::shared_ptr<storage<std::pair<K, V>>> db;


public:
  explicit recorder(std::shared_ptr<storage<std::pair<K, V>>> db) : db{db} {}

  ~recorder() = default;

  void produce(value_type data) {
    buffers[producer_idx.load()].write(data);
    if (buffers[producer_idx.load()].is_full())
      swap_buffers();
  }

  void consume() {
    while (running.load()) {
      if (!buffers[consumer_idx.load()].is_empty()) {
        db.get()->write(&buffers[consumer_idx.load()]);
        buffers[consumer_idx.load()].resize(0);
      }
    }
    db.get()->write(&buffers[producer_idx.load()]);
    buffers[producer_idx.load()].resize(0);
  }

  void start() {
    running.store(true);
  }

  void stop() {
    running.store(false);
  }

private:
  void swap_buffers() {
    producer_idx.store(producer_idx.load() ^ 1);
    consumer_idx.store(consumer_idx.load() ^ 1);
  }
};

#endif // UDES_REPLAY_RECORDER_H