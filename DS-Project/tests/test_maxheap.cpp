#include "../model/MaxHeap.h"
#include "../model/Validator.h"
#include "TestSupport.h"

static void checkHeapBuild(const int* values, int count, const char* message)
{
    IntArray input;
    for (int i = 0; i < count; i++)
    {
        input.push(values[i]);
    }
    MaxHeap heap;
    heap.buildMaxHeap(input);
    testCheck(heap.activeSize() == count, message);
    testCheck(heap.isMaxHeap(), "bottom-up construction satisfies max-heap order");
    testCheck(Validator::validateMaxHeap(heap), "heap build satisfies heap and sorted-region invariants");
}

void runMaxHeapTests()
{
    checkHeapBuild(nullptr, 0, "empty input builds an empty heap");
    const int one[] = { 7 };
    checkHeapBuild(one, 1, "one-element input builds a one-element heap");
    const int duplicates[] = { 4, 4, 9, 4, 9, 1, 1 };
    checkHeapBuild(duplicates, 7, "duplicate-heavy input builds a heap");
    const int alreadyHeap[] = { 20, 12, 18, 4, 9, 11, 17 };
    checkHeapBuild(alreadyHeap, 7, "already-heap input remains a heap");
    const int reversed[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    checkHeapBuild(reversed, 8, "reverse-heap input builds correctly");
    const int mixed[] = { 3, -8, 22, 5, 22, 0, -2, 14, 7 };
    checkHeapBuild(mixed, 9, "mixed and negative input builds correctly");

    IntArray input;
    for (int i = 0; i < 7; i++)
    {
        input.push(duplicates[i]);
    }
    MaxHeap heap;
    heap.buildMaxHeap(input);
    const int originalSize = heap.activeSize();
    int previous = 2147483647;
    while (heap.activeSize() > 0)
    {
        int extracted = 0;
        testCheck(heap.extractMax(extracted), "extractMax succeeds while the heap is nonempty");
        testCheck(extracted <= previous, "extractMax returns values in descending order");
        previous = extracted;
        testCheck(Validator::validateMaxHeap(heap), "heap remains valid after each extraction");
    }
    testCheck(heap.totalSize() == originalSize, "extraction preserves the backing array");
    testCheck(heap.isSortedAscending(), "repeated extraction leaves an ascending sorted region");

    MaxHeap empty;
    int noValue = 0;
    testCheck(!empty.extractMax(noValue), "extractMax reports empty heap");
    testCheck(!empty.swapIdx(0, 0), "heap rejects swaps outside the active range");
}
