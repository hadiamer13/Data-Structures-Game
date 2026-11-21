// model/EventLog.h

#pragma once

#include <string>
#include "../contracts/Types.h"
#include "../contracts/Event.h"
#include "../contracts/Containers.h"

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

    void record(const Event& e);

    void beginBatch();

    int currentStep() const;
    int size() const;
    bool empty() const;
    const Event& at(int i) const;
    const EventArray& events() const;

    IntPair batchRange(int step) const;

    void clear();

 
    int totalAlarmCost() const;
    int totalAlarmCost(int step) const;

    static Event inverse(const Event& e);
    static bool isExactlyInvertible(int eventType);

    EventArray inverseAll() const;

    static std::string describe(const Event& e);
    static const char* typeName(int eventType);

  private:
    EventArray m_events;
    int m_step;
};

inline void emit(EventLog* log, const Event& e)
{
    if (log != nullptr)
    {
        log->record(e);
    }
}
