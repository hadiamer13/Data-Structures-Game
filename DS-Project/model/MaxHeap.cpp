#include "MaxHeap.h"

MaxHeap::MaxHeap() : m_activeSize(0)
{
}

int MaxHeap::activeSize() const
{
    return m_activeSize;
}

int MaxHeap::totalSize() const
{
    return m_array.size();
}

const IntArray& MaxHeap::array() const
{
    return m_array;
}

void MaxHeap::swapIndices(int indexA, int indexB, int reason, EventLog* log)
{
    if (indexA == indexB)
    {
        return;
    }
    Event event(EventType::HEAP_SWAP);
    event.idx = indexA;
    event.idx2 = indexB;
    event.oldV = m_array[indexA];
    event.newV = m_array[indexB];
    event.result = reason;
    event.undo[0] = indexA;
    event.undo[1] = indexB;
    event.undo[2] = m_array[indexA];
    event.undo[3] = m_array[indexB];
    emit(log, event);

    const int temporary = m_array[indexA];
    m_array[indexA] = m_array[indexB];
    m_array[indexB] = temporary;
}

void MaxHeap::emitHeapSizeChange(int oldSize, int newSize, EventLog* log) const
{
    Event event(EventType::HEAP_SIZE_CHANGE);
    event.oldV = oldSize;
    event.newV = newSize;
    emit(log, event);
}

void MaxHeap::siftDownRaw(int index, int heapSize, int reason, EventLog* log)
{
    int current = index;
    while (current >= 0 && current < heapSize)
    {
        int largest = current;
        const int left = current * 2 + 1;
        const int right = current * 2 + 2;
        if (left < heapSize && m_array[left] > m_array[largest])
        {
            largest = left;
        }
        if (right < heapSize && m_array[right] > m_array[largest])
        {
            largest = right;
        }
        if (largest == current)
        {
            break;
        }
        swapIndices(current, largest, reason, log);
        current = largest;
    }
}

void MaxHeap::siftUpRaw(int index, int reason, EventLog* log)
{
    int current = index;
    while (current > 0)
    {
        const int parent = (current - 1) / 2;
        if (m_array[parent] >= m_array[current])
        {
            break;
        }
        swapIndices(parent, current, reason, log);
        current = parent;
    }
}

void MaxHeap::buildMaxHeap(const IntArray& values, EventLog* log)
{
    m_array = values;
    m_activeSize = m_array.size();
    buildMaxHeap(log);
}

void MaxHeap::buildMaxHeap(EventLog* log)
{
    m_activeSize = m_array.size();
    Event build(EventType::HEAP_BUILD);
    build.idx = m_activeSize / 2 - 1;
    build.idx2 = m_activeSize - 1;
    build.newV = m_activeSize;
    emit(log, build);

    for (int index = m_activeSize / 2 - 1; index >= 0; index--)
    {
        Event start(EventType::HEAPIFY_START);
        start.idx = index;
        start.newV = m_activeSize;
        emit(log, start);
        siftDownRaw(index, m_activeSize, SwapReason::Build, log);
        Event done(EventType::HEAPIFY_DONE);
        done.idx = index;
        done.newV = m_activeSize;
        emit(log, done);
    }
}

bool MaxHeap::siftDown(int index, int heapSize, EventLog* log)
{
    if (index < 0 || index >= heapSize || heapSize > m_activeSize)
    {
        return false;
    }
    Event start(EventType::HEAPIFY_START);
    start.idx = index;
    start.newV = heapSize;
    emit(log, start);
    siftDownRaw(index, heapSize, SwapReason::SiftDown, log);
    Event done(EventType::HEAPIFY_DONE);
    done.idx = index;
    done.newV = heapSize;
    emit(log, done);
    return true;
}

bool MaxHeap::siftUp(int index, EventLog* log)
{
    if (index < 0 || index >= m_activeSize)
    {
        return false;
    }
    siftUpRaw(index, SwapReason::SiftUp, log);
    return true;
}

bool MaxHeap::heapify(EventLog* log)
{
    if (m_activeSize < 0 || m_activeSize > m_array.size())
    {
        return false;
    }
    for (int index = m_activeSize / 2 - 1; index >= 0; index--)
    {
        Event start(EventType::HEAPIFY_START);
        start.idx = index;
        start.newV = m_activeSize;
        emit(log, start);
        siftDownRaw(index, m_activeSize, SwapReason::SiftDown, log);
        Event done(EventType::HEAPIFY_DONE);
        done.idx = index;
        done.newV = m_activeSize;
        emit(log, done);
    }
    return true;
}

bool MaxHeap::swapIdx(int indexA, int indexB, EventLog* log)
{
    if (indexA < 0 || indexB < 0 || indexA >= m_activeSize || indexB >= m_activeSize)
    {
        return false;
    }
    swapIndices(indexA, indexB, SwapReason::Player, log);
    return true;
}

bool MaxHeap::insert(int value, EventLog* log)
{
    if (m_activeSize < m_array.size())
    {
        // New values begin a fresh heap operation; the previous sorted suffix
        // becomes part of the input to that heap.
        buildMaxHeap(log);
    }
    const int oldSize = m_activeSize;
    if (m_activeSize == m_array.size())
    {
        m_array.push(value);
    }
    else
    {
        const int oldTotal = m_array.size();
        m_array.resize(oldTotal + 1);
        for (int index = oldTotal; index > m_activeSize; index--)
        {
            m_array[index] = m_array[index - 1];
        }
        m_array[m_activeSize] = value;
    }
    m_activeSize = m_activeSize + 1;
    emitHeapSizeChange(oldSize, m_activeSize, log);
    siftUpRaw(m_activeSize - 1, SwapReason::SiftUp, log);
    return true;
}

bool MaxHeap::extractMax(int& outValue, EventLog* log)
{
    if (m_activeSize <= 0)
    {
        return false;
    }

    const int oldSize = m_activeSize;
    const int last = oldSize - 1;
    outValue = m_array[0];
    if (last > 0)
    {
        swapIndices(0, last, SwapReason::Extract, log);
    }

    Event extracted(EventType::EXTRACT_MAX);
    extracted.idx = 0;
    extracted.idx2 = last;
    extracted.key = outValue;
    extracted.newV = oldSize;
    emit(log, extracted);

    m_activeSize = oldSize - 1;
    emitHeapSizeChange(oldSize, m_activeSize, log);

    Event sorted(EventType::MARK_SORTED);
    sorted.idx = last;
    sorted.key = outValue;
    emit(log, sorted);

    if (m_activeSize > 1)
    {
        siftDown(0, m_activeSize, log);
    }
    if (m_activeSize == 0)
    {
        Event complete(EventType::HEAP_SORT_COMPLETE);
        emit(log, complete);
    }
    return true;
}

int MaxHeap::extractMax(EventLog* log)
{
    int value = kNull;
    extractMax(value, log);
    return value;
}

void MaxHeap::heapSort(EventLog* log)
{
    buildMaxHeap(log);
    int ignored = 0;
    if (m_activeSize == 0)
    {
        Event complete(EventType::HEAP_SORT_COMPLETE);
        emit(log, complete);
        return;
    }
    while (m_activeSize > 0)
    {
        extractMax(ignored, log);
    }
}

bool MaxHeap::isMaxHeap() const
{
    return isMaxHeap(m_activeSize);
}

bool MaxHeap::isMaxHeap(int heapSize) const
{
    if (heapSize < 0 || heapSize > m_activeSize || heapSize > m_array.size())
    {
        return false;
    }
    for (int index = 0; index < heapSize; index++)
    {
        const int left = index * 2 + 1;
        const int right = index * 2 + 2;
        if (left < heapSize && m_array[index] < m_array[left])
        {
            return false;
        }
        if (right < heapSize && m_array[index] < m_array[right])
        {
            return false;
        }
    }
    return true;
}

bool MaxHeap::isSortedAscending() const
{
    if (m_activeSize < 0 || m_activeSize > m_array.size())
    {
        return false;
    }
    for (int index = m_activeSize + 1; index < m_array.size(); index++)
    {
        if (m_array[index - 1] > m_array[index])
        {
            return false;
        }
    }
    return true;
}
