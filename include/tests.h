#ifndef UDES_REPLAY_TESTS_H
#define UDES_REPLAY_TESTS_H

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

template<std::size_t N>
class snapshots {
public:
    using clock = std::chrono::high_resolution_clock;
    using precision = std::chrono::microseconds;

    struct metrics {
        double mean;
        double stddev;

        metrics& operator=(const metrics& oth) {
            this->stddev = oth.stddev;
            this->mean = oth.mean;
        }
    };

private:
    std::vector<precision> delays{};
    clock::time_point start = clock::now();

public:
    snapshots() {
        delays.reserve(N);
    }

    template<class F, class ... Args>
    void operator()(F f, Args &&... args) {
        auto pre = clock::now();
        f(std::forward<Args>(args)...);
        auto post = clock::now();
        delays.push_back(std::chrono::duration_cast<std::chrono::microseconds>(pre - start));
        start = post;
    }

    snapshots<N>& operator=(const snapshots<N>& oth) {
        this->delays = oth.snaps;
        this->start = oth.start;

        return *this;
    }

    snapshots<N> operator-(const snapshots<N>& oth) {
        snapshots<N> result{};

        std::transform(delays.begin(), delays.end(), oth.delays.begin(), std::back_inserter(result.delays),
                       [](const auto& lhs, const auto& rhs) {
                            auto diff = lhs - rhs;
                            return diff.count() < 0 ? -diff : diff;
                       });

        return result;
    }

private:
    metrics calc_metrics(const std::vector<precision>& data) const {
        std::vector<double> result{};

        std::transform(data.begin() + 1, data.end(), std::back_inserter(result),
                       [](const auto& t) { return t.count(); });

        double sum = std::accumulate(result.begin(), result.end(), 0.0);
        double mean = sum / static_cast<double>(result.size());

        double variance = 0.0;
        std::for_each(result.begin(), result.end(),
                      [mean, &variance](const auto& m) { variance += ((m - mean) * (m - mean)); });
        variance /= static_cast<double>(result.size());

        double stddev = std::sqrt(variance);
        return {  mean, stddev };
    }

public:
    metrics results() const {
        return { calc_metrics(delays) };
    }
};



#endif //UDES_REPLAY_TESTS_H
