#pragma once
// contracts/Types.h
// Shared basic types for model, game and gui.
// RULE: no SFML types in this file. Only plain C++.
// RULE: no namespace. Shared names use the Vh prefix to avoid clashes.

// ---- Constants -------------------------------------------------------
constexpr int kNull = -1;  // "no node" / "no index" (ids and indices)
constexpr int kLeafHeight = 1;   // Height convention: leaf = 1, empty tree = 0 (counted in nodes)

// ---- Own 2D point (replaces sf::Vector2f outside /gui) ----------------
struct VhPoint {
    float x = 0.0f;
    float y = 0.0f;
    VhPoint() = default;
    VhPoint(float px, float py) : x(px), y(py) {}
};

// ---- Which structure is being shown / edited -----------------------------
enum class TreeKind {
    Binary,    // Act 1: general binary tree
    BST,       // Act 2
    AVL,       // Act 3
    MaxHeap    // Act 4
};

// ---- Child side ----------------------------------------------------------------
enum class VhSide {
    None,      // root / not applicable
    Left,
    Right
};

// ---- AVL rotation cases -----------------------------------------------------
enum class RotationType {
    LL,        // right rotation
    RR,        // left rotation
    LR,        // left then right
    RL         // right then left
};

// ---- Subtree tint modes (HIGHLIGHT_SUBTREE event) -------------------------
enum class HighlightMode {
    Visited,
    Skipped,   // grey: pruned by range query
    Result
};

// ---- Result of a key comparison (COMPARE event) ----------------------------
enum class CompareResult {
    Less,      // key < node
    Equal,
    Greater
};