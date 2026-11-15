#include <cstdio>

#include "BenchSupport.h"
#include "../model/BST.h"
#include "../model/AVLTree.h"

static void rotationCounts(const IntArray& keys, std::uint64_t& insertTotal,
                           std::uint64_t& deleteTotal, int& maxDelete)
{
    AVLTree tree;
    EventLog log;
    for (int i = 0; i < keys.size(); i++)
    {
        tree.insert(keys[i], &log);
        insertTotal += static_cast<std::uint64_t>(countEvents(log, EventType::ROTATE));
        log.clear();
    }
    for (int i = keys.size() - 1; i >= 0; i--)
    {
        tree.remove(keys[i], &log);
        const int rotations = countEvents(log, EventType::ROTATE);
        deleteTotal += static_cast<std::uint64_t>(rotations);
        if (rotations > maxDelete) maxDelete = rotations;
        log.clear();
    }
}

static void timeRotations(const IntArray& keys, double& insertionMs, double& deletionMs)
{
    AVLTree tree;
    EventLog log;
    std::clock_t start = std::clock();
    for (int i = 0; i < keys.size(); i++)
    {
        tree.insert(keys[i], g_recordBenchEvents ? &log : nullptr);
        log.clear();
    }
    insertionMs = elapsedMilliseconds(start, std::clock());
    start = std::clock();
    for (int i = keys.size() - 1; i >= 0; i--)
    {
        tree.remove(keys[i], g_recordBenchEvents ? &log : nullptr);
        log.clear();
    }
    deletionMs = elapsedMilliseconds(start, std::clock());
}

void runRotationsKthBench()
{
    std::printf("\n[AVL rotations and order statistics]\n");
    std::printf("n,shape,avg_insert_rotations,avg_delete_rotations,max_rotations_per_delete,insert_ms,delete_ms\n");
    const int sizes[] = { 1000, 10000 };
    for (int s = 0; s < 2; s++)
    {
        for (int shape = 0; shape < 2; shape++)
        {
            IntArray keys;
            if (shape == 0)
            {
                for (int i = 0; i < sizes[s]; i++) keys.push(i + 1);
            }
            else makePermutation(sizes[s], 4403u + static_cast<std::uint32_t>(sizes[s]), keys);
            std::uint64_t inserted = 0;
            std::uint64_t deleted = 0;
            int maxDelete = 0;
            rotationCounts(keys, inserted, deleted, maxDelete);
            double insertMs = 0.0;
            double deleteMs = 0.0;
            timeRotations(keys, insertMs, deleteMs);
            std::printf("%d,%s,%.4f,%.4f,%d,%.3f,%.3f\n", sizes[s], shape == 0 ? "sorted" : "random",
                        static_cast<double>(inserted) / keys.size(), static_cast<double>(deleted) / keys.size(),
                        maxDelete, insertMs, deleteMs);
        }
    }

    std::printf("\n[nth order statistic: 1,000 queries]\n");
    std::printf("n,bst_inorder_ms,avl_kth_ms\n");
    for (int s = 0; s < 2; s++)
    {
        IntArray keys;
        makePermutation(sizes[s], 281u + static_cast<std::uint32_t>(sizes[s]), keys);
        BST bst;
        AVLTree avl;
        for (int i = 0; i < keys.size(); i++)
        {
            bst.insert(keys[i]);
            avl.insert(keys[i]);
        }
        volatile int sink = 0;
        std::clock_t start = std::clock();
        for (int query = 0; query < 1000; query++) sink += bst.kthSmallest((query * 31) % sizes[s] + 1);
        const double bstMs = elapsedMilliseconds(start, std::clock());
        start = std::clock();
        for (int query = 0; query < 1000; query++) sink += avl.kth((query * 31) % sizes[s] + 1);
        const double avlMs = elapsedMilliseconds(start, std::clock());
        std::printf("%d,%.3f,%.3f\n", sizes[s], bstMs, avlMs);
        (void)sink;
    }
}
