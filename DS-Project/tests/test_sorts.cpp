#include <algorithm>
#include "TestSupport.h"
#include "../model/Sorts.h"

static bool sortedByAllSorts(const int* input, int count)
{
    IntArray insertion;
    IntArray merge;
    IntArray quick;
    int expected[64];
    for (int i = 0; i < count; i++)
    {
        insertion.push(input[i]);
        merge.push(input[i]);
        quick.push(input[i]);
        expected[i] = input[i];
    }
    std::sort(expected, expected + count);
    SortStats insertionStats;
    SortStats mergeStats;
    SortStats quickStats;
    Sorts::insertionSort(insertion, &insertionStats);
    Sorts::mergeSort(merge, &mergeStats);
    Sorts::quickSort(quick, &quickStats);
    return sameInts(insertion.data(), expected, count) && sameInts(merge.data(), expected, count) &&
           sameInts(quick.data(), expected, count) &&
           (count < 2 || (insertionStats.comparisons > 0 && mergeStats.moves > 0 && quickStats.comparisons > 0));
}

void runSortTests()
{
    testCheck(sortedByAllSorts(nullptr, 0), "custom sorts handle empty input");
    const int one[] = { -4 };
    testCheck(sortedByAllSorts(one, 1), "custom sorts handle singleton input");
    const int duplicates[] = { 5, 2, 5, -1, 2, 0, -1, 5, 5 };
    testCheck(sortedByAllSorts(duplicates, 9), "custom sorts handle duplicates and negative values");
    const int sorted[] = { -3, -1, 0, 2, 8, 11, 15 };
    testCheck(sortedByAllSorts(sorted, 7), "custom sorts handle already sorted input");
    const int reversed[] = { 11, 8, 2, 0, -1, -3 };
    testCheck(sortedByAllSorts(reversed, 6), "custom sorts handle reversed input");
}
