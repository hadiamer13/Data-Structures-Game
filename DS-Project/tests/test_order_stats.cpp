#include <algorithm>
#include "../model/AVLTree.h"
#include "../model/Validator.h"
#include "TestSupport.h"

static void verifyOrderStatistics(AVLTree& tree, const int* keys, int count)
{
    int sorted[32];
    for (int i = 0; i < count; i++)
    {
        sorted[i] = keys[i];
    }
    std::sort(sorted, sorted + count);

    testCheck(tree.nodeCount() == count, "AVL node count matches the reference set");
    for (int k = 1; k <= count; k++)
    {
        testCheck(tree.kth(k) == sorted[k - 1], "AVL kth matches sorted-array reference");
    }
    int ignored = 0;
    testCheck(!tree.kth(0, ignored), "AVL rejects zero-based kth input");
    testCheck(!tree.kth(count + 1, ignored), "AVL rejects out-of-range kth input");

    for (int query = -5; query <= 25; query++)
    {
        int expectedRank = 0;
        for (int i = 0; i < count; i++)
        {
            if (sorted[i] <= query)
            {
                expectedRank = expectedRank + 1;
            }
        }
        testCheck(tree.rank(query) == expectedRank, "AVL rank matches sorted-array reference");
    }
    testCheck(tree.countInRange(4, 12) >= 0, "AVL range count is nonnegative");
    testCheck(tree.countInRange(12, 4) == 0, "AVL reversed range is empty");
    testCheck(Validator::validateAVL(tree), "AVL order-statistic metadata remains valid");
}

void runOrderStatisticTests()
{
    AVLTree tree;
    int values[32];
    int count = 0;
    const int inputs[] = {12, 5, 18, 2, 9, 15, 21, -3, 7, 11, 17, 1};
    for (int i = 0; i < 12; i++)
    {
        testCheck(tree.insert(inputs[i]), "AVL accepts a unique order-statistic key");
        values[count] = inputs[i];
        count = count + 1;
        verifyOrderStatistics(tree, values, count);
    }
    testCheck(!tree.insert(9), "AVL rejects duplicate keys");
    verifyOrderStatistics(tree, values, count);

    const int removals[] = {12, -3, 21, 5};
    for (int i = 0; i < 4; i++)
    {
        testCheck(tree.remove(removals[i]), "AVL removes an existing order-statistic key");
        int found = -1;
        for (int j = 0; j < count; j++)
        {
            if (values[j] == removals[i])
            {
                found = j;
                break;
            }
        }
        if (found >= 0)
        {
            for (int j = found; j < count - 1; j++)
            {
                values[j] = values[j + 1];
            }
            count = count - 1;
        }
        verifyOrderStatistics(tree, values, count);
    }
    testCheck(tree.countInRange(-3, 12) == 5, "AVL countInRange handles inclusive boundaries");
}
