#include <iostream>
#include <termkey.h>
#include <utility>


#include "api.h"
#include "preexisting-system.h"
#include "tests.h"

// =====================================================================================================================
// test de performance
// =====================================================================================================================
//int main() {
//  using data = preexisting_system::int_value;
//
//  enum { N = 1'000, CAP = 4096 };
//  snapshots<CAP> recording_snapshots{};
//
//  api<std::uint64_t, data, CAP> service{"data.bin"};
//  auto recorder = service.record();
//
//  preexisting_system::system<data, N> system{[&](auto data) {
//    recording_snapshots(recorder, data);
//  }};
//
//  service.stop();
//
//  snapshots<CAP> replaying_snapshots{};
//
//  auto replaying = [&](data d) {
//    auto print = [](const data &d) {};
//    replaying_snapshots(print, d);
//  };
//
//  service.replay(replaying);
//
//  auto delays = (recording_snapshots - replaying_snapshots).results();
//  std::cout << "moyenne=" << delays.mean << "us. ecart type=" << delays.stddev << "us." << std::endl;
//
//  return 0;
//}

// =====================================================================================================================
//
// =====================================================================================================================


bool is_escape(int);
bool is_key_pressed();
int read_blocking_key();
void clear();


#if defined(_WIN32)
// TODO
#else
#include <termkey.h>

TermKey *tk = termkey_new(0, TERMKEY_FLAG_UTF8);

TermKeyResult ret;
TermKeyKey key;

bool is_escape(int ch) {
    return key.type == TERMKEY_TYPE_KEYSYM && ch == 4;
}

bool is_key_pressed() {
    do { ret = termkey_waitkey(tk, &key); } while (ret == TERMKEY_RES_AGAIN);
    return true;
}

int read_blocking_key() {
    return key.code.number;
}

void clear() {
    termkey_destroy(tk);
}

#endif

int main() {
    api<std::uint64_t, int, 4096> service{ "storage.bin" };
    auto record = service.record();

    int ch;

    while (true) {
        if (is_key_pressed())
            ch = read_blocking_key();

        if (is_escape(ch))
            break;

        record(ch);
        std::cout << static_cast<char>(ch) << std::flush;
    }

    service.stop();

    std::cout << "\n\n voici la rediffusion: \n\n";

    auto produce = [](int v) { std::cout << static_cast<char>(v) << std::flush; };

    service.replay(produce);

    clear();

    return 0;
}
