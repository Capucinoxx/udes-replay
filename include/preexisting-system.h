#ifndef UDES_REPLAY_PREEXISTING_SYSTEM_H
#define UDES_REPLAY_PREEXISTING_SYSTEM_H

#include <atomic>
#include <thread>
#include <random>
#include <tuple>
#include <chrono>
#include <functional>

namespace preexisting_system {
    class int_value {
    public:
        int_value() = default;
        ~int_value() = default;

    public:
        int v = 0xdead;
    };

    template<class T, std::size_t N>
    class system {
    public:
        using value_type = T;
        using consumer = std::function<void(const value_type)>;

    private:
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<> dist{ 20, 50 };

        consumer consume;

    public:
        system(consumer consume) : consume { consume }, gen(rd()) {
            for (std::size_t i = 0; i != N; ++i) {
                consume(value_type{ });
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
            }
        }

        ~system() = default;
    };
}

#endif // UDES_REPLAY_PREEXISTING_SYSTEM_H
