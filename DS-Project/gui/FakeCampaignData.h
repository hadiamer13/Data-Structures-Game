// gui/FakeCampaignData.h
// TEMPORARY. Delete this file at integration.
// INTEGRATION (later): SaveManager supplies stars/unlocked,
// LevelManager/LevelLoader supply names and concepts.
// Index = act * 5 + level (0..19), so 2.3 is index 7.
#pragma once

namespace FakeCampaign {

    // 0 = not completed, 1..3 = best stars
    inline const int Stars[20] = {
        3, 2, 3, 1, 2,      // Act 1 all done
        3, 2, 1, 0, 0,      // Act 2: 2.3 done, 2.4 open, 2.5 locked
        0, 0, 0, 0, 0,      // Act 3
        0, 0, 0, 0, 0       // Act 4
    };

    // 1 = playable, 0 = locked
    inline const int Unlocked[20] = {
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    inline const char* const ActTitles[4] = {
        "ACT 1\nThe Blueprint Wing",
        "ACT 2\nThe Sorted Wing",
        "ACT 3\nThe Architect's Wing",
        "ACT 4\nThe Heap Fortress"
    };

    inline const char* const Names[20] = {
        "1.1 Guard Patrol", "1.2 Decode the Blueprint", "1.3 The Alarm Spreads",
        "1.4 Where Routes Meet", "1.5 Mirror the Vault",
        "2.1 Search Heist", "2.2 Stash Loot", "2.3 Steal and Collapse",
        "2.4 Range Raid", "2.5 The Lazy Architect",
        "3.1 Read the Badges", "3.2 Manual Repair: Singles",
        "3.3 Manual Repair: Doubles", "3.4 Auto Mode and Prediction",
        "3.5 Order-Statistic Heist",
        "4.1 Activate the Vault", "4.2 Repair the Security Locks",
        "4.3 Extract the Greatest Treasure", "4.4 Race Against the Guards",
        "4.5 Final Vault Boss"
    };

    inline const char* const Concepts[20] = {
        "Preorder, inorder, postorder, level order",
        "Rebuild a tree from two traversals",
        "Height via postorder, diameter",
        "LCA and path between nodes",
        "Mirror a tree by swapping children",
        "BST search, alarm counts comparisons",
        "BST insert",
        "BST delete: leaf, one child, two children",
        "Range query with subtree pruning",
        "Sorted inserts make a chain of height n",
        "AVL balance factors",
        "AVL single rotations: LL and RR",
        "AVL double rotations: LR and RL",
        "AVL insert with rebalancing",
        "Subtree size, kth smallest, rank",
        "Bottom-up Max Heap construction",
        "Heapify: sift-up and sift-down",
        "Extract max and shrink the heap",
        "Heap Sort under an operation limit",
        "Full Heap Sort workflow"
    };

    inline const char* const Objectives[20] = {
    "Guards patrol in a fixed order.\nClick the rooms in the order\nthe patrol will visit them.",
    "Rebuild the hidden tree from\nits traversals by dragging\nroom tokens into the slots.",
    "Enter every room's height,\nleaves first, then report\nthe diameter of the tree.",
    "Select two rooms, click the\nroom where their routes meet,\nthen trace the path.",
    "Swap the children of every\nroom, bottom-up, to mirror\nthe vault.",
    "Find the target treasure.\nEvery comparison costs\none alarm step.",
    "Stash the loot: choose the\npath and place the new room\nat the correct leaf.",
    "Remove the target room.\nPick the delete case first,\nthen watch it collapse.",
    "Collect every room in the\nrange [lo, hi]. Skip subtrees\nthat cannot hold a match.",
    "The vault is now a corridor.\nFind the target before the\nshrinking alarm fills.",
    "Every room shows a balance\nbadge. Click all of the\nunbalanced rooms.",
    "Repair the vault with a\nsingle rotation. Choose the\npivot and the direction.",
    "Identify LR or RL first,\nthen perform both rotations\nin the right order.",
    "Predict which rotation, if any,\nthe vault will perform\nbefore each insert.",
    "Steal the k-th most valuable\ntreasure in O(log n) steps.\nKeep every size correct.",
    "Turn the treasure array into\na valid Max Heap using\nbottom-up construction.",
    "A heap has been disturbed.\nRepair it with sift-up and\nsift-down swaps.",
    "Extract the maximum, restore\nthe heap, and move it into\nthe sorted region.",
    "Run several extract and\nheapify rounds before the\noperation limit runs out.",
    "Sort a brand new array in\nascending order. The door\nopens only when it is sorted."
    };

}  // namespace FakeCampaign