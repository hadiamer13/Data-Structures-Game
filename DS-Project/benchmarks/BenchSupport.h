#ifndef VH_BENCHSUPPORT_H
#define VH_BENCHSUPPORT_H

#include <cstdint>
#include <ctime>
#include "../contracts/Containers.h"
#include "../model/EventLog.h"

extern bool g_recordBenchEvents;

namespace BenchShape
{
    inline const int Random = 0;
    inline const int Sorted = 1;
    inline const int Reverse = 2;
    inline const int Duplicates = 3;
    inline const int Heap = 4;
}

struct BenchRng
{
    std::uint32_t state;

    explicit BenchRng(std::uint32_t seed) : state(seed == 0 ? 1 : seed)
    {
    }

    std::uint32_t next()
    {
        state = state * 1664525u + 1013904223u;
        return state;
    }

    int below(int limit)
    {
        return limit <= 0 ? 0 : static_cast<int>(next() % static_cast<std::uint32_t>(limit));
    }
};

inline void makePermutation(int count, std::uint32_t seed, IntArray& values)
{
    values.clear();
    for (int i = 0; i < count; i++) values.push(i + 1);
    BenchRng rng(seed);
    for (int i = count - 1; i > 0; i--)
    {
        const int j = rng.below(i + 1);
        const int temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
    }
}

inline void makeData(int count, int shape, std::uint32_t seed, IntArray& values)
{
    values.clear();
    BenchRng rng(seed);
    for (int i = 0; i < count; i++)
    {
        int value = i;
        if (shape == BenchShape::Reverse || shape == BenchShape::Heap) value = count - i;
        else if (shape == BenchShape::Random) value = rng.below(count * 8 + 1);
        else if (shape == BenchShape::Duplicates) value = rng.below(7);
        values.push(value);
    }
}

inline double elapsedMilliseconds(std::clock_t start, std::clock_t end)
{
    return 1000.0 * static_cast<double>(end - start) / static_cast<double>(CLOCKS_PER_SEC);
}

inline int countEvents(const EventLog& log, int type)
{
    int count = 0;
    for (int i = 0; i < log.size(); i++) if (log.at(i).type == type) count++;
    return count;
}

#endif
