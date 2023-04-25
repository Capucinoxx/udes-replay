#ifndef UDES_REPLAY_API_H
#define UDES_REPLAY_API_H

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <memory>

#include "storage.h"
#include "replayer.h"
#include "recorder.h"

template<typename K, typename V, std::size_t N>
class api {
public:
    using consumer_fn = std::function<void(V)>;

private:
    class timer {
    public:
        using clock = std::chrono::high_resolution_clock;

        std::pair<K, V> operator()(const V& data) noexcept {
            auto delay = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - start);
            start = clock::now();
            return { delay.count(), data };
        }
    private:
        clock::time_point start = clock::now();
    };

    timer t{ };
    std::shared_ptr<storage<std::pair<K, V>>> db;
    recorder<K, V, N> rec{  };
    replayer<K, V, N> rep{ };
    std::thread th1;
    std::thread th2;

public:
    explicit api(std::string storage_file)
        : db{ std::make_shared<storage<std::pair<K, V>>>(std::move(storage_file)) },
          rec{ db },
          rep{ db } {}
    ~api() = default;

    auto record() {
        rec.start();
        th1 = std::thread([&]() { rec.consume(); });
        return [&](V data) { rec.produce(t(data)); };
    }

    void stop() {
        rec.stop();

        th1.join();
    }

    void replay(consumer_fn consume) {
        rep.preload();
        th1 = std::thread([&]() { rep.consume(consume); });
        th2 = std::thread([&]() { rep.produce(); });
        th2.join();
        th1.join();

    }
};

#endif // UDES_REPLAY_API_H