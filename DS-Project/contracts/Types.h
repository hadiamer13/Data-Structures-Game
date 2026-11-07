#pragma once
// contracts/Types.h
// No enums, no global variables. Named constants live inside structs.
// RULE: no SFML types here. Never compare against raw numbers, use the names.

struct VhConst {
    static inline const int Null = -1;  // "no node" / "no index"
    static inline const int LeafHeight = 1;   // leaf = 1, empty tree = 0 (counted in nodes)
};

struct VhPoint {
    float x = 0.0f;
    float y = 0.0f;
    VhPoint() = default;
    VhPoint(float px, float py) : x(px), y(py) {}
};

struct TreeKind {
    static inline const int Binary = 0;  // Act 1
    static inline const int BST = 1;  // Act 2
    static inline const int AVL = 2;  // Act 3
    static inline const int MaxHeap = 3;  // Act 4
};

struct VhSide {
    static inline const int None = 0;    // root / not applicable
    static inline const int Left = 1;
    static inline const int Right = 2;
};

struct RotationType {
    static inline const int LL = 0;       // right rotation
    static inline const int RR = 1;       // left rotation
    static inline const int LR = 2;       // left then right
    static inline const int RL = 3;       // right then left
};

struct HighlightMode {
    static inline const int Visited = 0;
    static inline const int Skipped = 1;  // grey: pruned by range query
    static inline const int Result = 2;
};

struct CompareResult {
    static inline const int Less = 0;  // key < node
    static inline const int Equal = 1;
    static inline const int Greater = 2;
};