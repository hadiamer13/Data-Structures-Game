#ifndef VH_MAXHEAP_H
#define VH_MAXHEAP_H

#include <cstdint>
#include "../contracts/Containers.h"
#include "../contracts/Event.h"
#include "EventLog.h"

// Zero-based max heap. array()[0..activeSize()) is the heap; the remaining
// suffix is the ascending sorted region produced by repeated extractMax().
class MaxHeap
{
public:
    MaxHeap();

    int activeSize() const;
    int totalSize() const;
    const IntArray& array() const;

    void buildMaxHeap(const IntArray& values, EventLog* log = nullptr);
    void buildMaxHeap(EventLog* log = nullptr);
    bool siftDown(int index, int heapSize, EventLog* log = nullptr);
    bool siftUp(int index, EventLog* log = nullptr);
    bool heapify(EventLog* log = nullptr);
    bool swapIdx(int indexA, int indexB, EventLog* log = nullptr);
    bool insert(int value, EventLog* log = nullptr);
    bool extractMax(int& outValue, EventLog* log = nullptr);
    int extractMax(EventLog* log = nullptr);
    void heapSort(EventLog* log = nullptr);
    bool loadState(const IntArray& values, int activeSize);
    std::uint64_t comparisonCount() const;
    std::uint64_t swapCount() const;

    bool isMaxHeap() const;
    bool isMaxHeap(int heapSize) const;
    bool isSortedAscending() const;
    bool loadRaw(const IntArray& values, int activeSize);

private:
    void siftDownRaw(int index, int heapSize, int reason, EventLog* log);
    void siftUpRaw(int index, int reason, EventLog* log);
    void swapIndices(int indexA, int indexB, int reason, EventLog* log);
    void emitHeapSizeChange(int oldSize, int newSize, EventLog* log) const;

    IntArray m_array;
    int m_activeSize;
    std::uint64_t m_comparisons;
    std::uint64_t m_swaps;
};

#endif
