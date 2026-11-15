#ifndef VH_TESTSUPPORT_H
#define VH_TESTSUPPORT_H

#include "../third_party/doctest/doctest.h"

inline void testCheck(bool condition, const char* message)
{
    CHECK_MESSAGE(condition, message);
}

inline bool sameInts(const int* left, const int* right, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (left[i] != right[i])
        {
            return false;
        }
    }
    return true;
}

#endif
