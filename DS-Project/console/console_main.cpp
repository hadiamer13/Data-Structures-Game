#include <cstdio>

#include "../model/AVLTree.h"
#include "../model/BST.h"
#include "../model/MaxHeap.h"
#include "../model/Sorts.h"

int main()
{
    const int values[] = { 8, 3, 10, 1, 6, 14, 4, 7, 13 };
    BST bst;
    AVLTree avl;
    for (int i = 0; i < 9; i++)
    {
        bst.insert(values[i]);
        avl.insert(values[i]);
    }

    IntArray heapValues;
    IntArray sortedValues;
    for (int i = 0; i < 9; i++)
    {
        heapValues.push(values[i]);
        sortedValues.push(values[i]);
    }
    MaxHeap heap;
    heap.buildMaxHeap(heapValues);
    heap.heapSort();
    SortStats stats;
    Sorts::mergeSort(sortedValues, &stats);

    std::printf("BST nodes: %d\nAVL height: %d (balanced: %s)\n",
                bst.nodeCount(), avl.height(), avl.isBalanced() ? "yes" : "no");
    std::printf("Max-heap sorted size: %d\nMerge sort comparisons: %llu\n",
                heap.totalSize(), static_cast<unsigned long long>(stats.comparisons));
    return avl.isBalanced() && heap.isSortedAscending() ? 0 : 1;
}
