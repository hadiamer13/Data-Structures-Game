// model/EventLog.cpp
#include "EventLog.h"
#include <sstream>

EventLog::EventLog() : m_step(0)
{
}

void EventLog::record(const Event& e)
{
    Event copy = e;
    copy.step = m_step;
    m_events.push(copy);
}

void EventLog::beginBatch()
{
    m_step = m_step + 1;
}

int EventLog::currentStep() const
{
    return m_step;
}

int EventLog::size() const
{
    return m_events.size();
}

bool EventLog::empty() const
{
    return m_events.empty();
}

const Event& EventLog::at(int i) const
{
    return m_events[i];
}

const EventArray& EventLog::events() const
{
    return m_events;
}

IntPair EventLog::batchRange(int step) const
{
    int first = kNull;
    int last = kNull;
    for (int i = 0; i < m_events.size(); i++)
    {
        if (m_events[i].step == step)
        {
            if (first == kNull)
            {
                first = i;
            }
            last = i;
        }
    }
    return IntPair(first, last);
}

void EventLog::clear()
{
    m_events.clear();
    m_step = 0;
}

int EventLog::totalAlarmCost() const
{
    int total = 0;
    for (int i = 0; i < m_events.size(); i++)
    {
        total = total + m_events[i].alarmCost;
    }
    return total;
}

int EventLog::totalAlarmCost(int step) const
{
    int total = 0;
    for (int i = 0; i < m_events.size(); i++)
    {
        if (m_events[i].step == step)
        {
            total = total + m_events[i].alarmCost;
        }
    }
    return total;
}

bool EventLog::isExactlyInvertible(int t)
{
    return t == EventType::INSERT || t == EventType::UNLINK || t == EventType::UPDATE_HEIGHT ||
           t == EventType::UPDATE_SIZE || t == EventType::HEAP_SIZE_CHANGE ||
           t == EventType::SWAP_VALUE || t == EventType::HEAP_SWAP;
}

Event EventLog::inverse(const Event& e)
{
    Event r = e;
    if (e.type == EventType::INSERT)
    {
        r.type = EventType::UNLINK;
    }
    else if (e.type == EventType::UNLINK)
    {
        r.type = EventType::INSERT;
    }
    else if (e.type == EventType::UPDATE_HEIGHT || e.type == EventType::UPDATE_SIZE ||
             e.type == EventType::HEAP_SIZE_CHANGE)
    {
        r.oldV = e.newV;
        r.newV = e.oldV;
    }
    // SWAP_VALUE and HEAP_SWAP: swapping the same pair again undoes it, so r == e.
    // Everything else (ROTATE, ...): unchanged copy; undo uses snapshot restore.
    r.alarmCost = 0;  // undoing never charges the alarm meter
    return r;
}

EventArray EventLog::inverseAll() const
{
    EventArray out;
    for (int i = m_events.size() - 1; i >= 0; i--)
    {
        out.push(inverse(m_events[i]));
    }
    return out;
}

const char* EventLog::typeName(int t)
{
    if (t == EventType::VISIT)
    {
        return "VISIT";
    }
    if (t == EventType::COMPARE)
    {
        return "COMPARE";
    }
    if (t == EventType::INSERT)
    {
        return "INSERT";
    }
    if (t == EventType::UNLINK)
    {
        return "UNLINK";
    }
    if (t == EventType::SWAP_VALUE)
    {
        return "SWAP_VALUE";
    }
    if (t == EventType::ROTATE)
    {
        return "ROTATE";
    }
    if (t == EventType::UPDATE_HEIGHT)
    {
        return "UPDATE_HEIGHT";
    }
    if (t == EventType::UPDATE_SIZE)
    {
        return "UPDATE_SIZE";
    }
    if (t == EventType::BALANCE_ALERT)
    {
        return "BALANCE_ALERT";
    }
    if (t == EventType::HIGHLIGHT_SUBTREE)
    {
        return "HIGHLIGHT_SUBTREE";
    }
    if (t == EventType::HEAP_BUILD)
    {
        return "HEAP_BUILD";
    }
    if (t == EventType::HEAP_SWAP)
    {
        return "HEAP_SWAP";
    }
    if (t == EventType::HEAPIFY_START)
    {
        return "HEAPIFY_START";
    }
    if (t == EventType::HEAPIFY_DONE)
    {
        return "HEAPIFY_DONE";
    }
    if (t == EventType::EXTRACT_MAX)
    {
        return "EXTRACT_MAX";
    }
    if (t == EventType::MARK_SORTED)
    {
        return "MARK_SORTED";
    }
    if (t == EventType::HEAP_SIZE_CHANGE)
    {
        return "HEAP_SIZE_CHANGE";
    }
    if (t == EventType::HEAP_SORT_COMPLETE)
    {
        return "HEAP_SORT_COMPLETE";
    }
    if (t == EventType::DONE)
    {
        return "DONE";
    }
    return "UNKNOWN";
}

static const char* sideName(int s)
{
    if (s == Side::Left)
    {
        return "left";
    }
    if (s == Side::Right)
    {
        return "right";
    }
    return "-";
}

static const char* rotName(int r)
{
    if (r == RotationType::LL)
    {
        return "LL";
    }
    if (r == RotationType::RR)
    {
        return "RR";
    }
    if (r == RotationType::LR)
    {
        return "LR";
    }
    if (r == RotationType::RL)
    {
        return "RL";
    }
    return "none";
}

std::string EventLog::describe(const Event& e)
{
    std::ostringstream s;
    int t = e.type;

    if (t == EventType::VISIT)
    {
        s << "Visit room " << e.id << " (alarm +" << e.alarmCost << ")";
    }
    else if (t == EventType::COMPARE)
    {
        const char* dir = (e.result < 0) ? "go left" : ((e.result > 0) ? "go right" : "found");
        s << "Compare " << e.key << " with room " << e.id << ": " << dir << " (alarm +" << e.alarmCost << ")";
    }
    else if (t == EventType::INSERT)
    {
        s << "Insert room " << e.id << " under " << e.id2 << " (" << sideName(e.side) << ")";
    }
    else if (t == EventType::UNLINK)
    {
        s << "Unlink room " << e.id << " from " << e.id2 << " (" << sideName(e.side) << ")";
    }
    else if (t == EventType::SWAP_VALUE)
    {
        s << "Swap values of rooms " << e.id << " and " << e.id2 << " (" << e.oldV << " <-> " << e.newV << ")";
    }
    else if (t == EventType::ROTATE)
    {
        s << "Rotate " << rotName(e.result) << " at room " << e.id;
    }
    else if (t == EventType::UPDATE_HEIGHT)
    {
        s << "Height of room " << e.id << ": " << e.oldV << " -> " << e.newV;
    }
    else if (t == EventType::UPDATE_SIZE)
    {
        s << "Size of room " << e.id << ": " << e.oldV << " -> " << e.newV;
    }
    else if (t == EventType::BALANCE_ALERT)
    {
        s << "Imbalance at room " << e.id << " (balance factor " << e.newV << ")";
    }
    else if (t == EventType::HIGHLIGHT_SUBTREE)
    {
        const char* m = (e.result == HighlightMode::Skipped) ? "skipped" : ((e.result == HighlightMode::Result) ? "result" : "visited");
        s << "Subtree at room " << e.id << ": " << m;
    }
    else if (t == EventType::HEAP_BUILD)
    {
        s << "Build heap: sift indices " << e.idx << " down to " << e.idx2 << " (n = " << e.newV << ")";
    }
    else if (t == EventType::HEAP_SWAP)
    {
        s << "Swap array cells " << e.idx << " and " << e.idx2 << " (" << e.oldV << " <-> " << e.newV << ")";
    }
    else if (t == EventType::HEAPIFY_START)
    {
        s << "Heapify from index " << e.idx << " (heap size " << e.newV << ")";
    }
    else if (t == EventType::HEAPIFY_DONE)
    {
        s << "Heapify done at index " << e.idx << " (heap size " << e.newV << ")";
    }
    else if (t == EventType::EXTRACT_MAX)
    {
        s << "Extract maximum " << e.key << " from index " << e.idx << " (heap size " << e.newV << ")";
    }
    else if (t == EventType::MARK_SORTED)
    {
        s << "Mark index " << e.idx << " sorted (value " << e.key << ")";
    }
    else if (t == EventType::HEAP_SIZE_CHANGE)
    {
        s << "Heap size " << e.oldV << " -> " << e.newV;
    }
    else if (t == EventType::HEAP_SORT_COMPLETE)
    {
        s << "Heap sort complete: Final Vault Unlocked";
    }
    else if (t == EventType::DONE)
    {
        s << "Done (alarm " << e.key << ")";
    }
    else
    {
        s << "Unknown event " << t;
    }
    return s.str();
}
