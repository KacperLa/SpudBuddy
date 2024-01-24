#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <cstdint>

inline std::int64_t get_time_micro()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

#endif // COMMON_H
