#ifndef VH_TESTSUPPORT_H
#define VH_TESTSUPPORT_H

#include <cstdio>

inline int vhTestAssertions = 0;
inline int vhTestFailures = 0;

inline void testCheck(bool condition, const char* message)
{
    vhTestAssertions = vhTestAssertions + 1;
    if (!condition)
    {
        vhTestFailures = vhTestFailures + 1;
        std::printf("FAIL: %s\n", message);
    }
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
