// contracts/Event.h
// One plain struct Event (frozen decision 1). Model emits these; GUI plays them back.
// Field meaning per event type is documented below. Unused fields stay kNull / 0.
#ifndef VH_EVENT_H
#define VH_EVENT_H

#include "Types.h"

namespace EventType
{
    inline const int VISIT = 0;              // id                         alarm +1
    inline const int COMPARE = 1;            // id, key, result            alarm +1
    inline const int INSERT = 2;             // id, id2=parentId, side
    inline const int UNLINK = 3;             // id, id2=parentId, side
    inline const int SWAP_VALUE = 4;         // id, id2, oldV=key of id, newV=key of id2
    inline const int ROTATE = 5;             // id=pivot, result=RotationType
    inline const int UPDATE_HEIGHT = 6;      // id, oldV, newV
    inline const int UPDATE_SIZE = 7;        // id, oldV, newV
    inline const int BALANCE_ALERT = 8;      // id, newV=balance factor
    inline const int HIGHLIGHT_SUBTREE = 9;  // id, result=HighlightMode
    inline const int HEAP_BUILD = 10;        // idx=first index to sift, idx2=last index, newV=array size
    inline const int HEAP_SWAP = 11;         // idx, idx2, oldV=A[idx] before, newV=A[idx2] before, result=reason
    inline const int HEAPIFY_START = 12;     // idx=start index, newV=heapSize
    inline const int HEAPIFY_DONE = 13;      // idx=start index, newV=heapSize
    inline const int EXTRACT_MAX = 14;       // idx=rootIndex, key=value, newV=heapSize
    inline const int MARK_SORTED = 15;       // idx, key=value
    inline const int HEAP_SIZE_CHANGE = 16;  // oldV, newV
    inline const int HEAP_SORT_COMPLETE = 17;// (final array is read from the snapshot)
    inline const int DONE = 18;              // ends a batch; key = total alarm cost of batch
    inline const int Count = 19;
}

// Reasons for HEAP_SWAP (stored in Event::result).
namespace SwapReason
{
    inline const int SiftUp = 0;
    inline const int SiftDown = 1;
    inline const int Extract = 2;   // root <-> last active element
    inline const int Player = 3;    // manual swap in Act 4
    inline const int Build = 4;
}

// Size of the undo payload. 10 ints fit a ROTATE inverse
// (pivot's parent + side, pivot/child old links, two old heights, two old sizes).
inline const int kUndoSize = 10;

struct Event
{
    int type;       // EventType::*
    int id;         // primary node id (kNull if none)
    int id2;        // secondary node id (parent, swap partner, ...)
    int side;       // Side::*
    int idx;        // primary heap array index
    int idx2;       // secondary heap array index
    int key;        // key / value involved
    int result;     // CompareResult / RotationType / HighlightMode / SwapReason
    int oldV;       // previous value (heights, sizes, array values)
    int newV;       // new value
    int undo[kUndoSize]; // data needed to invert this event (layout per type, owned by model)
    int alarmCost;  // set by the model: how much this event adds to the alarm meter
    int step;       // action batch number; auto-filled by EventLog (never set by model ops)

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
