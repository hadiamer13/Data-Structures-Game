#pragma once
// contracts/Event.h
// One plain event struct, no variant, no SFML.
// Every model operation records VhEvents into an EventLog (nullptr = off).

#include "Types.h"

enum class EventType {
    // --- Trees (Acts 1-3) ---
    VISIT,              // id                         alarm +1
    COMPARE,            // id, key, result            alarm +1
    INSERT,             // id, id2 = parent, side
    UNLINK,             // id, id2 = parent, side
    SWAP_VALUE,         // id, id2                    (delete two-child case)
    ROTATE,             // id = pivot, rotation
    UPDATE_HEIGHT,      // id, oldV, newV
    UPDATE_SIZE,        // id, oldV, newV
    BALANCE_ALERT,      // id, oldV = balance factor
    HIGHLIGHT_SUBTREE,  // id, mode

    // --- Heap (Act 4) ---
    HEAP_BUILD,         // idx = first index to sift, idx2 = heap size
    HEAP_SWAP,          // idx, idx2, oldV = value at idx, newV = value at idx2 (before swap)
    HEAPIFY_START,      // idx = start index, oldV = heap size
    HEAPIFY_DONE,       // idx = start index, oldV = heap size
    EXTRACT_MAX,        // key = value, idx = root index, oldV = heap size
    MARK_SORTED,        // idx, key = value
    HEAP_SIZE_CHANGE,   // oldV, newV
    HEAP_SORT_COMPLETE, // end of heap sort

    // --- Common ---
    DONE                // ends the batch
};

struct VhEvent {
    EventType     type = EventType::DONE;

    int           id = kNull;   // primary node id
    int           id2 = kNull;   // second node id (parent, swap partner)
    VhSide        side = VhSide::None;

    int           idx = kNull;   // heap array index
    int           idx2 = kNull;   // second heap array index

    int           key = 0;       // key or value involved
    CompareResult result = CompareResult::Equal;   // COMPARE only
    RotationType  rotation = RotationType::LL;       // ROTATE only
    HighlightMode mode = HighlightMode::Visited; // HIGHLIGHT_SUBTREE only

    int           oldV = 0;       // old value (height, size, bf, heap value)
    int           newV = 0;       // new value

    int           undo[6] = { 0, 0, 0, 0, 0, 0 };  // extra data to reverse the event
    int           alarmCost = 0;       // set by the model: 1 for VISIT/COMPARE, else 0
};