// model/EventLog.h
// Ordered list of events produced by model operations.
// Every model op takes EventLog* log; nullptr turns recording off (benchmarks).
// Always record through emit(log, e), which is null-safe.
#pragma once

#include <string>
#include "../contracts/Types.h"
#include "../contracts/Event.h"
#include "../contracts/Containers.h"

// Default alarm cost for an event type (model fills Event::alarmCost with this).
// VISIT and COMPARE cost 1; everything else is free unless a level adds a penalty.
inline int defaultAlarmCost(int eventType)
{
    if (eventType == EventType::VISIT || eventType == EventType::COMPARE)
    {
        return 1;
    }
    return 0;
}

class EventLog
{
public:
    EventLog();

    // Stores a copy of e, stamping e.step with the current batch number.
    void record(const Event& e);

    // Starts a new batch (one player action = one batch). Called by GameSession before each op.
    void beginBatch();

    int currentStep() const;
    int size() const;
    bool empty() const;
    const Event& at(int i) const;
    const EventArray& events() const;

    // Index range [first, last] of the events in one batch, or first = kNull when absent.
    IntPair batchRange(int step) const;

    // Drops all events and restarts batch numbering at 0.
    void clear();

    // Sum of alarmCost over all events (or over one batch).
    int totalAlarmCost() const;
    int totalAlarmCost(int step) const;

    // Inverse of one event. Simple types are exactly invertible:
    //   INSERT <-> UNLINK, UPDATE_HEIGHT / UPDATE_SIZE / HEAP_SIZE_CHANGE swap oldV/newV,
    //   SWAP_VALUE and HEAP_SWAP are self-inverse.
    // ROTATE and the remaining types are returned as an unchanged copy: undo for those is done
    // by snapshot restore (frozen decision 6), not by inverse events.
    static Event inverse(const Event& e);
    static bool isExactlyInvertible(int eventType);

    // All events inverted, in reverse order (apply front to back to undo).
    EventArray inverseAll() const;

    // Human-readable one-line text for the Event Log panel.
    static std::string describe(const Event& e);
    static const char* typeName(int eventType);

private:
    EventArray m_events;
    int m_step;
};

// Null-safe recording helper: emit(log, e) does nothing when log is nullptr.
inline void emit(EventLog* log, const Event& e)
{
    if (log != nullptr)
    {
        log->record(e);
    }
}
