#include <cstdio>

#include "BenchSupport.h"
#include "../model/MaxHeap.h"

void runHeapBench()
{
    std::printf("\n[Max Heap build and Heap Sort]\n");
    std::printf("n,shape,build_ms,build_comparisons,build_swaps,heapsort_ms,heapsort_comparisons,heapsort_swaps\n");
    const int sizes[] = { 100, 1000, 10000 };
    const int shapes[] = { BenchShape::Random, BenchShape::Sorted, BenchShape::Reverse, BenchShape::Duplicates };
    const char* names[] = { "random", "sorted", "reverse", "duplicates" };

    for (int s = 0; s < 3; s++)
    {
        for (int sh = 0; sh < 4; sh++)
        {
            IntArray data;
            makeData(sizes[s], shapes[sh], 719u + static_cast<std::uint32_t>(sizes[s]) + static_cast<std::uint32_t>(sh), data);

            // Build only: measured on its own heap.
            MaxHeap builder;
            EventLog log;
            std::clock_t start = std::clock();
            builder.buildMaxHeap(data, g_recordBenchEvents ? &log : nullptr);
            const double buildMs = elapsedMilliseconds(start, std::clock());
            const std::uint64_t buildComparisons = builder.comparisonCount();
            const std::uint64_t buildSwaps = builder.swapCount();

            // Heap sort: starts from the raw input, so its counts include the real build.
            MaxHeap sorter;
            sorter.loadRaw(data, data.size());
            log.clear();
            start = std::clock();
            sorter.heapSort(g_recordBenchEvents ? &log : nullptr);
            const double sortMs = elapsedMilliseconds(start, std::clock());

            std::printf("%d,%s,%.3f,%llu,%llu,%.3f,%llu,%llu\n", sizes[s], names[sh], buildMs,
                static_cast<unsigned long long>(buildComparisons), static_cast<unsigned long long>(buildSwaps), sortMs,
                static_cast<unsigned long long>(sorter.comparisonCount()),
                static_cast<unsigned long long>(sorter.swapCount()));
        }
    }
}
