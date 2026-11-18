// gui/EventText.h
// One-line text for an Event (Event Log panel). Header-only, no SFML.
// INTEGRATION (optional): model/EventLog::describe() can replace it.
#pragma once
#include <string>
#include "../contracts/Event.h"
#include "../contracts/Types.h"

inline std::string describeEventText(const Event& e)
{
    int t = e.type;
    std::string s;
    if (t == EventType::VISIT)
    {
        s = "Visit room #" + std::to_string(e.id);
    }
    else if (t == EventType::COMPARE)
    {
        const char* dir = (e.result < 0) ? "go left" : ((e.result > 0) ? "go right" : "found");
        s = "Compare " + std::to_string(e.key) + " with #" + std::to_string(e.id) + ": " + dir;
    }
    else if (t == EventType::INSERT)
    {
        s = "Insert room #" + std::to_string(e.id);
    }
    else if (t == EventType::UNLINK)
    {
        s = "Unlink room #" + std::to_string(e.id);
    }
    else if (t == EventType::SWAP_VALUE)
    {
        s = "Swap values of #" + std::to_string(e.id) + " and #" + std::to_string(e.id2);
    }
    else if (t == EventType::ROTATE)
    {
        const char* r = "?";
        if (e.result == RotationType::LL)
        {
            r = "LL";
        }
        else if (e.result == RotationType::RR)
        {
            r = "RR";
        }
        else if (e.result == RotationType::LR)
        {
            r = "LR";
        }
        else if (e.result == RotationType::RL)
        {
            r = "RL";
        }
        s = std::string("Rotate ") + r + " at pivot #" + std::to_string(e.id);
    }
    else if (t == EventType::UPDATE_HEIGHT)
    {
        s = "Height of #" + std::to_string(e.id) + ": " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
    }
    else if (t == EventType::UPDATE_SIZE)
    {
        s = "Size of #" + std::to_string(e.id) + ": " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
    }
    else if (t == EventType::BALANCE_ALERT)
    {
        s = "BALANCE ALERT at #" + std::to_string(e.id) + " (bf " + std::to_string(e.newV) + ")";
    }
    else if (t == EventType::HIGHLIGHT_SUBTREE)
    {
        s = std::string((e.result == HighlightMode::Skipped) ? "Skip" : "Mark") + " subtree at #" + std::to_string(e.id);
    }
    else if (t == EventType::HEAP_BUILD)
    {
        s = "Build heap, array size " + std::to_string(e.newV);
    }
    else if (t == EventType::HEAP_SWAP)
    {
        s = "Swap cells [" + std::to_string(e.idx) + "] and [" + std::to_string(e.idx2) + "]";
    }
    else if (t == EventType::HEAPIFY_START)
    {
        s = "Heapify from [" + std::to_string(e.idx) + "], heap size " + std::to_string(e.newV);
    }
    else if (t == EventType::HEAPIFY_DONE)
    {
        s = "Heapify done: heap property restored";
    }
    else if (t == EventType::EXTRACT_MAX)
    {
        s = "Extract max " + std::to_string(e.key) + " (heap size " + std::to_string(e.newV) + ")";
    }
    else if (t == EventType::MARK_SORTED)
    {
        s = "Value " + std::to_string(e.key) + " moved to sorted region";
    }
    else if (t == EventType::HEAP_SIZE_CHANGE)
    {
        s = "Heap size " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
    }
    else if (t == EventType::HEAP_SORT_COMPLETE)
    {
        s = "HEAP SORT COMPLETE";
    }
    else if (t == EventType::DONE)
    {
        s = "Done";
    }
    else
    {
        s = "Event " + std::to_string(t);
    }
    return s;
}
