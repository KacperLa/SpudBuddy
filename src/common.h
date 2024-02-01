#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <cstdint>

using namespace std::chrono;

// inline std::int64_t get_time_micro()
// {
//     return std::chrono::duration_cast<std::chrono::microseconds>(
//         std::chrono::steady_clock::now().time_since_epoch()).count();
// }

inline std::int64_t get_time_micro()
{
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

#endif // COMMON_H
