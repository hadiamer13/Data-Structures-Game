
#ifndef VH_EVENT_H
#define VH_EVENT_H

#include "Types.h"

namespace EventType
{
    inline const int VISIT = 0;
    inline const int COMPARE = 1;
    inline const int INSERT = 2;
    inline const int UNLINK = 3;
    inline const int SWAP_VALUE = 4;
    inline const int ROTATE = 5;
    inline const int UPDATE_HEIGHT = 6;
    inline const int UPDATE_SIZE = 7;
    inline const int BALANCE_ALERT = 8;
    inline const int HIGHLIGHT_SUBTREE = 9;
    inline const int HEAP_BUILD = 10;
    inline const int HEAP_SWAP = 11;
    inline const int HEAPIFY_START = 12;
    inline const int HEAPIFY_DONE = 13;
    inline const int EXTRACT_MAX = 14;
    inline const int MARK_SORTED = 15;
    inline const int HEAP_SIZE_CHANGE = 16;
    inline const int HEAP_SORT_COMPLETE = 17;
    inline const int DONE = 18;
    inline const int Count = 19;
}

namespace SwapReason
{
    inline const int SiftUp = 0;
    inline const int SiftDown = 1;
    inline const int Extract = 2;
    inline const int Player = 3;
    inline const int Build = 4;
}

inline const int kUndoSize = 10;

struct Event
{
    int type;
    int id;
    int id2;
    int side;
    int idx;
    int idx2;
    int key;
    int result;
    int oldV;
    int newV;
    int undo[kUndoSize];
    int alarmCost;
    int step;

    Event()
    {
        type = kNull;
        id = kNull;
        id2 = kNull;
        side = Side::None;
        idx = kNull;
        idx2 = kNull;
        key = 0;
        result = 0;
        oldV = 0;
        newV = 0;

        for (int i = 0; i < kUndoSize; i++)
        {
            undo[i] = 0;
        }

        alarmCost = 0;
        step = 0;
    }

    explicit Event(int t)
    {
        *this = Event();
        type = t;
    }
};

#endif
