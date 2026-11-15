#include <cstdio>

#include "BenchSupport.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"
#include "../model/BinaryTree.h"

static int iterativeHeight(const TreeNode* root)
{
    if (root == nullptr) return 0;
    NodeQueue queue;
    queue.push(const_cast<TreeNode*>(root));
    int height = 0;
    while (!queue.empty())
    {
        int width = queue.size();
        height++;
        while (width-- > 0)
        {
            TreeNode* node = queue.front();
            queue.pop();
            if (node->left != nullptr) queue.push(node->left);
            if (node->right != nullptr) queue.push(node->right);
        }
    }
    return height;
}

static void buildTrees(const IntArray& keys, BST& bst, AVLTree& avl, EventLog& log)
{
    for (int i = 0; i < keys.size(); i++)
    {
        EventLog* sink = g_recordBenchEvents ? &log : nullptr;
        bst.insert(keys[i], sink);
        avl.insert(keys[i], sink);
        log.clear();
    }
}

static void searchCounts(const BST& bst, const AVLTree& avl, const IntArray& queries,
                         std::uint64_t& bstComparisons, std::uint64_t& avlComparisons)
{
    EventLog log;
    for (int i = 0; i < queries.size(); i++)
    {
        bst.search(queries[i], &log);
        bstComparisons += static_cast<std::uint64_t>(log.size());
        log.clear();
        avl.find(queries[i], &log);
        avlComparisons += static_cast<std::uint64_t>(log.size());
        log.clear();
    }
}

static void timeSearches(const BST& bst, const AVLTree& avl, const IntArray& queries,
                         double& bstMs, double& avlMs)
{
    EventLog log;
    std::clock_t start = std::clock();
    for (int i = 0; i < queries.size(); i++)
    {
        bst.search(queries[i], g_recordBenchEvents ? &log : nullptr);
        log.clear();
    }
    bstMs = elapsedMilliseconds(start, std::clock());
    start = std::clock();
    for (int i = 0; i < queries.size(); i++)
    {
        avl.find(queries[i], g_recordBenchEvents ? &log : nullptr);
        log.clear();
    }
    avlMs = elapsedMilliseconds(start, std::clock());
}

void runHeightSearchBench()
{
    std::printf("\n[Height and 10,000 searches]\n");
    std::printf("n,shape,bst_height,avl_height,bst_build_ms,avl_build_ms,bst_avg_comparisons,avl_avg_comparisons,bst_search_ms,avl_search_ms\n");
    const int sizes[] = { 100, 1000, 10000 };
    for (int s = 0; s < 3; s++)
    {
        for (int shape = 0; shape < 2; shape++)
        {
            IntArray keys;
            if (shape == 0)
            {
                for (int i = 0; i < sizes[s]; i++) keys.push(i + 1);
            }
            else makePermutation(sizes[s], 7351u + static_cast<std::uint32_t>(sizes[s]), keys);
            BST bst;
            AVLTree avl;
            EventLog log;
            std::clock_t start = std::clock();
            for (int i = 0; i < keys.size(); i++)
            {
                bst.insert(keys[i], g_recordBenchEvents ? &log : nullptr);
                log.clear();
            }
            const double bstBuild = elapsedMilliseconds(start, std::clock());
            start = std::clock();
            for (int i = 0; i < keys.size(); i++)
            {
                avl.insert(keys[i], g_recordBenchEvents ? &log : nullptr);
                log.clear();
            }
            const double avlBuild = elapsedMilliseconds(start, std::clock());
            IntArray queries;
            BenchRng queryRng(92311u + static_cast<std::uint32_t>(sizes[s]));
            for (int i = 0; i < 10000; i++) queries.push(queryRng.below(sizes[s]) + 1);
            std::uint64_t bstComparisons = 0;
            std::uint64_t avlComparisons = 0;
            searchCounts(bst, avl, queries, bstComparisons, avlComparisons);
            double bstSearch = 0.0;
            double avlSearch = 0.0;
            timeSearches(bst, avl, queries, bstSearch, avlSearch);
            std::printf("%d,%s,%d,%d,%.3f,%.3f,%.2f,%.2f,%.3f,%.3f\n",
                        sizes[s], shape == 0 ? "sorted" : "random", iterativeHeight(bst.root()),
                        avl.height(), bstBuild, avlBuild, static_cast<double>(bstComparisons) / queries.size(),
                        static_cast<double>(avlComparisons) / queries.size(), bstSearch, avlSearch);
        }
    }
}
