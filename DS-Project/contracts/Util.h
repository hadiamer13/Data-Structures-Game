// contracts/Util.h
// Own helpers replacing <algorithm>. Header-only, inline, no templates.
#ifndef VH_UTIL_H
#define VH_UTIL_H

inline int minI(int a, int b)
{
    return (a < b) ? a : b;
}

inline int maxI(int a, int b)
{
    return (a > b) ? a : b;
}

inline int absI(int a)
{
    return (a < 0) ? -a : a;
}

inline int clampI(int v, int lo, int hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

inline void swapInt(int& a, int& b)
{
    int t = a;
    a = b;
    b = t;
}

inline float minF(float a, float b)
{
    return (a < b) ? a : b;
}

inline float maxF(float a, float b)
{
    return (a > b) ? a : b;
}

inline float absF(float a)
{
    return (a < 0.f) ? -a : a;
}

inline float clampF(float v, float lo, float hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

inline void swapFloat(float& a, float& b)
{
    float t = a;
    a = b;
    b = t;
}

#endif
