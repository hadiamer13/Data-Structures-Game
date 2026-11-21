#include <algorithm>
#include "../model/MaxHeap.h"
#include "../model/Validator.h"
#include "TestSupport.h"

static void checkHeapSort(const int* values, int count, const char* message)
{
    IntArray input;
    int expected[32];
    for (int i = 0; i < count; i++)
    {
        input.push(values[i]);
        expected[i] = values[i];
    }
    std::sort(expected, expected + count);

    MaxHeap heap;
    heap.buildMaxHeap(input);
    heap.heapSort();
    testCheck(heap.activeSize() == 0, message);
    testCheck(heap.totalSize() == count, "heap sort retains the array length");
    testCheck(heap.isSortedAscending(), "heap sort output is ascending");
    testCheck(Validator::validateMaxHeap(heap), "sorted heap state passes validator");
    for (int i = 0; i < count; i++)
    {
        testCheck(heap.array()[i] == expected[i], "heap sort matches trusted sorted reference");
    }
}

void runHeapSortTests()
{
    checkHeapSort(nullptr, 0, "heap sort handles empty input");
    const int singleton[] = {-4};
    checkHeapSort(singleton, 1, "heap sort handles a singleton");
    const int duplicates[] = {5, 2, 5, -1, 2, 0, -1};
    checkHeapSort(duplicates, 7, "heap sort handles duplicates and negatives");
    const int sorted[] = {-3, -1, 0, 2, 8, 11};
    checkHeapSort(sorted, 6, "heap sort handles sorted input");
    const int reversed[] = {11, 8, 2, 0, -1, -3};
    checkHeapSort(reversed, 6, "heap sort handles reverse-sorted input");
    const int randomLike[] = {6, -5, 19, 0, 2, 11, -2, 8, 3};
    checkHeapSort(randomLike, 9, "heap sort handles mixed input");
}
