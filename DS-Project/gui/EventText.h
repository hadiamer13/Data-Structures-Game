// gui/EventText.h
// Event ka aik line ka text (Event Log panel ke liye). Header-only hai, SFML nahi chahiye.
// Chaaho to model/EventLog::describe() bhi istemal ho sakta hai.
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
        s = "Room #" + std::to_string(e.id) + " par visit";
    }
    else if (t == EventType::COMPARE)
    {
        const char* dir = (e.result < 0) ? "go left" : ((e.result > 0) ? "go right" : "found");
        s = "Compare " + std::to_string(e.key) + " with #" + std::to_string(e.id) + ": " + dir;
    }
    else if (t == EventType::INSERT)
    {
        s = "Room #" + std::to_string(e.id) + " insert hua";
    }
    else if (t == EventType::UNLINK)
    {
        s = "Room #" + std::to_string(e.id) + " unlink hua";
    }
    else if (t == EventType::SWAP_VALUE)
    {
        s = "#" + std::to_string(e.id) + " aur #" + std::to_string(e.id2) + " ki values swap";
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
        s = std::string("Rotate ") + r + " pivot #" + std::to_string(e.id) + " par";
    }
    else if (t == EventType::UPDATE_HEIGHT)
    {
        s = "#" + std::to_string(e.id) + " ki height: " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
    }
    else if (t == EventType::UPDATE_SIZE)
    {
        s = "#" + std::to_string(e.id) + " ka size: " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
    }
    else if (t == EventType::BALANCE_ALERT)
    {
        s = "BALANCE ALERT #" + std::to_string(e.id) + " par (bf " + std::to_string(e.newV) + ")";
    }
    else if (t == EventType::HIGHLIGHT_SUBTREE)
    {
        s = std::string((e.result == HighlightMode::Skipped) ? "Skip" : "Mark") + " subtree #" + std::to_string(e.id) + " par";
    }
    else if (t == EventType::HEAP_BUILD)
    {
        s = "Heap banana, array size " + std::to_string(e.newV);
    }
    else if (t == EventType::HEAP_SWAP)
    {
        s = "Cells [" + std::to_string(e.idx) + "] aur [" + std::to_string(e.idx2) + "] swap";
    }
    else if (t == EventType::HEAPIFY_START)
    {
        s = "[" + std::to_string(e.idx) + "] se heapify, heap size " + std::to_string(e.newV);
    }
    else if (t == EventType::HEAPIFY_DONE)
    {
        s = "Heapify khatam: heap property restore ho gayi";
    }
    else if (t == EventType::EXTRACT_MAX)
    {
        s = "Extract max " + std::to_string(e.key) + " (heap size " + std::to_string(e.newV) + ")";
    }
    else if (t == EventType::MARK_SORTED)
    {
        s = "Value " + std::to_string(e.key) + " sorted region mein chali gayi";
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