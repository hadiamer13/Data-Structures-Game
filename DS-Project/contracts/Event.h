#pragma once
// contracts/Event.h
// One plain event struct. Every model operation records VhEvents into an EventLog (nullptr = off).

#include "contracts/Types.h"

struct EventType {
    // --- Trees (Acts 1-3) ---
    static inline const int VISIT = 0;   // id                      alarm +1
    static inline const int COMPARE = 1;   // id, key, result         alarm +1
    static inline const int INSERT = 2;   // id, id2 = parent, side
    static inline const int UNLINK = 3;   // id, id2 = parent, side
    static inline const int SWAP_VALUE = 4;   // id, id2
    static inline const int ROTATE = 5;   // id = pivot, rotation
    static inline const int UPDATE_HEIGHT = 6;   // id, oldV, newV
    static inline const int UPDATE_SIZE = 7;   // id, oldV, newV
    static inline const int BALANCE_ALERT = 8;   // id, oldV = balance factor
    static inline const int HIGHLIGHT_SUBTREE = 9;   // id, mode

    // --- Heap (Act 4) ---
    static inline const int HEAP_BUILD = 10;  // idx = first index to sift, idx2 = heap size
    static inline const int HEAP_SWAP = 11;  // idx, idx2, oldV = value at idx, newV = value at idx2 (before swap)
    static inline const int HEAPIFY_START = 12;  // idx = start index, oldV = heap size
    static inline const int HEAPIFY_DONE = 13;  // idx = start index, oldV = heap size
    static inline const int EXTRACT_MAX = 14;  // key = value, idx = root index, oldV = heap size
    static inline const int MARK_SORTED = 15;  // idx, key = value
    static inline const int HEAP_SIZE_CHANGE = 16;  // oldV, newV
    static inline const int HEAP_SORT_COMPLETE = 17;  // end of heap sort

    // --- Common ---
    static inline const int DONE = 18;  // ends the batch
};

struct VhEvent {
    int type = EventType::DONE;

    int id = VhConst::Null;        // primary node id
    int id2 = VhConst::Null;        // second node id (parent, swap partner)
    int side = VhSide::None;

    int idx = VhConst::Null;        // heap array index
    int idx2 = VhConst::Null;        // second heap array index

    int key = 0;                    // key or value involved
    int result = CompareResult::Equal;     // COMPARE only
    int rotation = RotationType::LL;         // ROTATE only
    int mode = HighlightMode::Visited;   // HIGHLIGHT_SUBTREE only

    int oldV = 0;                    // old value (height, size, bf, heap value)
    int newV = 0;                    // new value

    int undo[6] = { 0, 0, 0, 0, 0, 0 };   // extra data to reverse the event
    int alarmCost = 0;                    // set by the model: 1 for VISIT/COMPARE, else 0
};