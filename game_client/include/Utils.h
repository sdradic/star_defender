#pragma once
#include <chrono>
#include <thread>
#include <cstdlib>

inline void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline int random_int(int min, int max) {
    return min + rand() % ((max+1)-min);
}