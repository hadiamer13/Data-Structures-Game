#include <cstdio>

#include "BenchSupport.h"
#include "../model/MaxHeap.h"
#include "../model/Sorts.h"

static double timeInsertion(const IntArray& source, int repetitions, SortStats& total)
{
    const std::clock_t start = std::clock();
    for (int r = 0; r < repetitions; r++)
    {
        IntArray values = source;
        SortStats one;
        Sorts::insertionSort(values, &one);
        total.comparisons += one.comparisons;
        total.swaps += one.swaps;
        total.moves += one.moves;
    }
    return elapsedMilliseconds(start, std::clock()) / repetitions;
}

static double timeMerge(const IntArray& source, int repetitions, SortStats& total)
{
    const std::clock_t start = std::clock();
    for (int r = 0; r < repetitions; r++)
    {
        IntArray values = source;
        SortStats one;
        Sorts::mergeSort(values, &one);
        total.comparisons += one.comparisons;
        total.moves += one.moves;
    }
    return elapsedMilliseconds(start, std::clock()) / repetitions;
}

static double timeQuick(const IntArray& source, int repetitions, SortStats& total)
{
    const std::clock_t start = std::clock();
    for (int r = 0; r < repetitions; r++)
    {
        IntArray values = source;
        SortStats one;
        Sorts::quickSort(values, &one);
        total.comparisons += one.comparisons;
        total.swaps += one.swaps;
        total.moves += one.moves;
    }
    return elapsedMilliseconds(start, std::clock()) / repetitions;
}

static double timeHeap(const IntArray& source, int repetitions, std::uint64_t& comparisons,
                       std::uint64_t& swaps)
{
    const std::clock_t start = std::clock();
    for (int r = 0; r < repetitions; r++)
    {
        MaxHeap heap;
        EventLog log;
        heap.buildMaxHeap(source, g_recordBenchEvents ? &log : nullptr);
        while (heap.activeSize() > 0)
        {
            heap.extractMax(g_recordBenchEvents ? &log : nullptr);
        }
        comparisons += heap.comparisonCount();
        swaps += heap.swapCount();
    }
    return elapsedMilliseconds(start, std::clock()) / repetitions;
}

void runSortingBench()
{
    std::printf("\n[Heap Sort vs insertion, merge and quick sort]\n");
    std::printf("n,shape,algorithm,avg_ms,avg_comparisons,avg_swaps,auxiliary_space\n");
    const int sizes[] = { 100, 1000, 10000 };
    const int shapes[] = { BenchShape::Random, BenchShape::Sorted, BenchShape::Reverse, BenchShape::Duplicates };
    const char* names[] = { "random", "sorted", "reverse", "duplicates" };
    for (int s = 0; s < 3; s++)
    {
        const int repetitions = sizes[s] == 100 ? 5 : (sizes[s] == 1000 ? 2 : 1);
        for (int sh = 0; sh < 4; sh++)
        {
            IntArray source;
            makeData(sizes[s], shapes[sh], 719u + static_cast<std::uint32_t>(sizes[s]) + static_cast<std::uint32_t>(sh), source);
            SortStats stats;
            double milliseconds = timeInsertion(source, repetitions, stats);
            std::printf("%d,%s,insertion,%.3f,%.0f,%.0f,O(1)\n", sizes[s], names[sh], milliseconds,
                static_cast<double>(stats.comparisons) / repetitions, static_cast<double>(stats.swaps) / repetitions);
            stats = SortStats();
            milliseconds = timeMerge(source, repetitions, stats);
            std::printf("%d,%s,merge,%.3f,%.0f,%.0f,O(n)\n", sizes[s], names[sh], milliseconds,
                static_cast<double>(stats.comparisons) / repetitions, static_cast<double>(stats.swaps) / repetitions);
            stats = SortStats();
            milliseconds = timeQuick(source, repetitions, stats);
            std::printf("%d,%s,quick,%.3f,%.0f,%.0f,O(log n) avg\n", sizes[s], names[sh], milliseconds,
                static_cast<double>(stats.comparisons) / repetitions, static_cast<double>(stats.swaps) / repetitions);
            std::uint64_t comparisons = 0;
            std::uint64_t swaps = 0;
            milliseconds = timeHeap(source, repetitions, comparisons, swaps);
            std::printf("%d,%s,heap_sort,%.3f,%llu,%llu,O(1)\n", sizes[s], names[sh], milliseconds,
                static_cast<unsigned long long>(comparisons / repetitions),
                static_cast<unsigned long long>(swaps / repetitions));
        }
    }
}
