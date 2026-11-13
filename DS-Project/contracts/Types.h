// contracts/Types.h
// Shared basic types and int-constant "enums". No SFML, no STL containers.
#ifndef VH_TYPES_H
#define VH_TYPES_H

// Sentinel for "no node / no index / not applicable".
inline const int kNull = -1;

// Height convention (frozen decision 4): height counts NODES.
// Empty tree = 0, leaf = 1.
inline const int kEmptyHeight = 0;
inline const int kLeafHeight = 1;

// Plain 2D point (layout math lives in model-independent code; no sf::Vector2f here).
struct Point
{
    float x;
    float y;

    Point() : x(0.f), y(0.f)
    {
    }

    Point(float px, float py) : x(px), y(py)
    {
    }
};

// Small replacement for std::pair<int,int>.
struct IntPair
{
    int a;
    int b;

    IntPair() : a(0), b(0)
    {
    }

    IntPair(int pa, int pb) : a(pa), b(pb)
    {
    }
};

namespace TreeKind
{
    inline const int Binary = 0;
    inline const int BST = 1;
    inline const int AVL = 2;
    inline const int MaxHeap = 3;
}

namespace Side
{
    inline const int None = -1;
    inline const int Left = 0;
    inline const int Right = 1;
}

namespace RotationType
{
    inline const int None = -1;
    inline const int LL = 0;
    inline const int RR = 1;
    inline const int LR = 2;
    inline const int RL = 3;
}

namespace HighlightMode
{
    inline const int Visited = 0;
    inline const int Skipped = 1;
    inline const int Result = 2;
}

namespace CompareResult
{
    inline const int Less = -1;
    inline const int Equal = 0;
    inline const int Greater = 1;
}

#endif
