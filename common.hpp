#ifndef HACKMATCH_COMMON_HPP
#define HACKMATCH_COMMON_HPP

#include <chrono>
#include <iostream>

namespace HackMatch {
class Timer {
    const std::chrono::high_resolution_clock::time_point t0;
    const char* message;
public:
    Timer(const char* message) : t0(std::chrono::high_resolution_clock::now()), message(message) {}
    ~Timer() {
        const auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << message << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << '\n';
    }
};
}
#endif
