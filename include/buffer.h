#ifndef UDES_REPLAY_BUFFER_H
#define UDES_REPLAY_BUFFER_H

#include <iostream>
#include <array>
#include <atomic>

template<typename T, std::size_t N>
class buffer {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const reference;
    using pointer = value_type*;
    using array_pointer = std::array<value_type, N>::pointer;
    using iterator = pointer;

    [[nodiscard]] iterator begin() noexcept        { return values.begin(); }
    [[nodiscard]] iterator end() noexcept          { return values.begin() + this->size(); }
    array_pointer data() noexcept                  { return values.data(); }
    [[nodiscard]] std::size_t size() const         { return n_elems.load(); }
    [[nodiscard]] std::size_t cap() const noexcept { return N; }

private:
    std::array<value_type, N> values { };
    std::atomic<std::size_t> n_elems = 0;

public:
    buffer() = default;
    ~buffer() = default;

    void write(const_reference v) { values[n_elems.fetch_add(1)] = v; }
    void resize(std::size_t n) { n_elems.store(n); }

    bool is_full()  { return n_elems.load() == N; }
    bool is_empty() { return n_elems.load() == 0; }
};

#endif // UDES_REPLAY_BUFFER_H