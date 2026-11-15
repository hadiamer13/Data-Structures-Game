#ifndef VH_SORTS_H
#define VH_SORTS_H

#include <cstdint>
#include "../contracts/Containers.h"

struct SortStats
{
    std::uint64_t comparisons;
    std::uint64_t swaps;
    std::uint64_t moves;

    SortStats() : comparisons(0), swaps(0), moves(0)
    {
    }
};

// In-place comparison sorts over the project's concrete IntArray container.
class Sorts
{
public:
    static void insertionSort(IntArray& values, SortStats* stats = nullptr);
    static void mergeSort(IntArray& values, SortStats* stats = nullptr);
    static void quickSort(IntArray& values, SortStats* stats = nullptr);

private:
    static void mergeSortRange(IntArray& values, int* scratch, int first, int last, SortStats* stats);
    static void quickSortRange(IntArray& values, int first, int last, SortStats* stats);
    static void swapValues(int& left, int& right, SortStats* stats);
};

#endif
