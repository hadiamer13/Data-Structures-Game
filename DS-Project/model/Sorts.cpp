#include "Sorts.h"

void Sorts::insertionSort(IntArray& values, SortStats* stats)
{
    for (int i = 1; i < values.size(); i++)
    {
        const int value = values[i];
        int j = i;
        while (j > 0)
        {
            if (stats != nullptr)
            {
                stats->comparisons++;
            }
            if (values[j - 1] <= value)
            {
                break;
            }
            values[j] = values[j - 1];
            if (stats != nullptr)
            {
                stats->moves++;
            }
            j--;
        }
        values[j] = value;
        if (stats != nullptr && j != i)
        {
            stats->moves++;
        }
    }
}

void Sorts::mergeSortRange(IntArray& values, int* scratch, int first, int last, SortStats* stats)
{
    if (last - first < 2)
    {
        return;
    }
    const int middle = first + (last - first) / 2;
    mergeSortRange(values, scratch, first, middle, stats);
    mergeSortRange(values, scratch, middle, last, stats);
    int left = first;
    int right = middle;
    int out = first;
    while (left < middle && right < last)
    {
        if (stats != nullptr)
        {
            stats->comparisons++;
        }
        if (values[left] <= values[right])
        {
            scratch[out++] = values[left++];
        }
        else
        {
            scratch[out++] = values[right++];
        }
        if (stats != nullptr)
        {
            stats->moves++;
        }
    }
    while (left < middle)
    {
        scratch[out++] = values[left++];
        if (stats != nullptr)
        {
            stats->moves++;
        }
    }
    while (right < last)
    {
        scratch[out++] = values[right++];
        if (stats != nullptr)
        {
            stats->moves++;
        }
    }
    for (int i = first; i < last; i++)
    {
        values[i] = scratch[i];
        if (stats != nullptr)
        {
            stats->moves++;
        }
    }
}

void Sorts::mergeSort(IntArray& values, SortStats* stats)
{
    if (values.size() < 2)
    {
        return;
    }
    IntArray scratch;
    scratch.resize(values.size());
    mergeSortRange(values, scratch.data(), 0, values.size(), stats);
}

void Sorts::swapValues(int& left, int& right, SortStats* stats)
{
    if (left == right)
    {
        return;
    }
    const int temporary = left;
    left = right;
    right = temporary;
    if (stats != nullptr)
    {
        stats->swaps++;
        stats->moves += 3;
    }
}

void Sorts::quickSortRange(IntArray& values, int first, int last, SortStats* stats)
{
    int low = first;
    int high = last;
    while (low < high)
    {
        int left = low;
        int right = high;
        const int pivot = values[low + (high - low) / 2];
        for (;;)
        {
            while (left <= high)
            {
                if (stats != nullptr)
                {
                    stats->comparisons++;
                }
                if (values[left] >= pivot)
                {
                    break;
                }
                left++;
            }
            while (right >= low)
            {
                if (stats != nullptr)
                {
                    stats->comparisons++;
                }
                if (values[right] <= pivot)
                {
                    break;
                }
                right--;
            }
            if (left >= right)
            {
                break;
            }
            swapValues(values[left], values[right], stats);
            left++;
            right--;
        }
        const int split = right;
        if (split - low < high - split)
        {
            if (low < split)
            {
                quickSortRange(values, low, split, stats);
            }
            low = split + 1;
        }
        else
        {
            if (split + 1 < high)
            {
                quickSortRange(values, split + 1, high, stats);
            }
            high = split;
        }
    }
}

void Sorts::quickSort(IntArray& values, SortStats* stats)
{
    if (values.size() > 1)
    {
        quickSortRange(values, 0, values.size() - 1, stats);
    }
}
