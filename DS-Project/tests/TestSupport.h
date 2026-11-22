#ifndef VH_TESTSUPPORT_H
#define VH_TESTSUPPORT_H

#include "../third_party/doctest/doctest.h"

// Evaluate the condition into a plain bool first. doctest then receives a single
// bool, so it never sees the && or || operators it forbids.
// Macro, not a function, so the failure line points at the test that failed.
#define testCheck(condition, message)                              \
    do                                                             \
    {                                                              \
        const bool testCheckValue = static_cast<bool>(condition);  \
        CHECK_MESSAGE(testCheckValue, message);                    \
    } while (false)

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