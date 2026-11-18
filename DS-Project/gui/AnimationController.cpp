// gui/AnimationController.cpp
#include "AnimationController.h"

namespace
{
    bool snapshotBeforeBegin(int t)
    {
        return t == EventType::INSERT || t == EventType::ROTATE;
    }

    bool snapshotAfterBegin(int t)
    {
        return t == EventType::UNLINK;
    }
}

AnimationController::AnimationController(IEventVisualizer* visualizer)
    : m_vis(visualizer), m_items(nullptr), m_count(0), m_cap(0),
    m_hasStart(false), m_hasEnd(false),
    m_index(0), m_t(0.f), m_speed(1.f),
    m_busy(false), m_paused(false), m_active(false), m_stepping(false),
    m_stepReq(false), m_skipping(false),
    m_onEvent(nullptr), m_onDone(nullptr), m_cbContext(nullptr)
{
}

AnimationController::~AnimationController()
{
    freeItems();
    delete[] m_items;
}

void AnimationController::setCallbacks(AnimEventCallback onEvent, AnimDoneCallback onDone, void* context)
{
    m_onEvent = onEvent;
    m_onDone = onDone;
    m_cbContext = context;
}

float AnimationController::eventDuration(int type)
{
    if (type == EventType::VISIT || type == EventType::COMPARE)
    {
        return 0.5f;
    }
    if (type == EventType::ROTATE)
    {
        return 1.0f;
    }
    if (type == EventType::HEAP_SWAP || type == EventType::SWAP_VALUE)
    {
        return 0.7f;
    }
    if (type == EventType::INSERT || type == EventType::UNLINK)
    {
        return 0.6f;
    }
    if (type == EventType::DONE)
    {
        return 0.05f;
    }
    return 0.4f;
}

// ------------------------------------------------------------ batch building

void AnimationController::freeItems()
{
    for (int i = 0; i < m_count; i++)
    {
        delete m_items[i].after;
        m_items[i].after = nullptr;
    }
    m_count = 0;
}

void AnimationController::clear()
{
    freeItems();
    m_index = 0;
    m_t = 0.f;
    m_busy = false;
    m_active = false;
    m_stepping = false;
    m_stepReq = false;
    m_skipping = false;
    m_hasStart = false;
    m_hasEnd = false;
}

void AnimationController::beginBatch(const ViewSnapshot& before)
{
    clear();
    m_start = before;
    m_hasStart = true;
}

void AnimationController::enqueue(const Event& e, const ViewSnapshot* afterEvent)
{
    if (m_count == m_cap)
    {
        int newCap = (m_cap == 0) ? 16 : m_cap * 2;
        AnimItem* nd = new AnimItem[newCap];
        for (int i = 0; i < m_count; i++)
        {
            nd[i] = m_items[i];
        }
        delete[] m_items;   // item snapshot pointers were copied, not deleted
        m_items = nd;
        m_cap = newCap;
    }
    m_items[m_count].ev = e;
    m_items[m_count].after = (afterEvent != nullptr) ? new ViewSnapshot(*afterEvent) : nullptr;
    m_count++;
}

void AnimationController::endBatch(const ViewSnapshot& afterAll)
{
    m_end = afterAll;
    m_hasEnd = true;
    startPlayback();
}

void AnimationController::startPlayback()
{
    m_index = 0;
    m_t = 0.f;
    m_active = false;
    m_stepping = false;
    m_stepReq = false;
    m_skipping = false;
    m_paused = false;
    m_busy = (m_count > 0);
    if (!m_busy)
    {
        // nothing to animate: just show the result
        finishBatch();
    }
}

// ------------------------------------------------------------------ playback

void AnimationController::beginCurrent()
{
    AnimItem& it = m_items[m_index];
    int t = it.ev.type;
    bool snap = (it.after != nullptr) && !m_skipping;

    if (snap && snapshotBeforeBegin(t))
    {
        m_vis->animateTo(*it.after);
    }
    m_vis->beginEvent(it.ev);
    if (snap && snapshotAfterBegin(t))
    {
        m_vis->animateTo(*it.after);
    }

    m_t = 0.f;
    m_active = true;
    if (m_onEvent)
    {
        m_onEvent(m_cbContext, it.ev, m_index, m_count);
    }
}

void AnimationController::completeCurrent()
{
    AnimItem& it = m_items[m_index];
    int t = it.ev.type;

    m_vis->progress(it.ev, 1.f);
    m_vis->endEvent(it.ev);
    if (it.after != nullptr && !m_skipping && !snapshotBeforeBegin(t) && !snapshotAfterBegin(t))
    {
        m_vis->animateTo(*it.after);
    }

    m_active = false;
    m_stepping = false;
    m_t = 0.f;
    m_index++;
    if (m_index >= m_count)
    {
        finishBatch();
    }
}

void AnimationController::finishBatch()
{
    m_busy = false;
    m_active = false;
    m_stepping = false;
    m_stepReq = false;
    if (m_hasEnd)
    {
        if (m_skipping)
        {
            m_vis->snapTo(m_end);
        }
        else
        {
            m_vis->animateTo(m_end);
        }
    }
    m_skipping = false;
    if (m_onDone)
    {
        m_onDone(m_cbContext);
    }
}

void AnimationController::update(float dt)
{
    if (!m_busy)
    {
        return;
    }

    if (!m_active)
    {
        if (m_paused && !m_stepReq)
        {
            return;
        }
        if (m_stepReq)
        {
            m_stepping = true;
            m_stepReq = false;
        }
        beginCurrent();
        return;
    }

    if (m_paused && !m_stepping)
    {
        return;
    }

    AnimItem& it = m_items[m_index];
    m_t += dt * m_speed;
    float d = eventDuration(it.ev.type);
    if (m_t < d)
    {
        m_vis->progress(it.ev, m_t / d);
        return;
    }
    completeCurrent();
}

// ------------------------------------------------------------------ controls

void AnimationController::play()
{
    m_paused = false;
}

void AnimationController::pause()
{
    m_paused = true;
}

void AnimationController::togglePause()
{
    m_paused = !m_paused;
}

void AnimationController::stepOne()
{
    if (!m_busy)
    {
        return;
    }
    m_paused = true;
    if (m_active)
    {
        m_stepping = true;   // let the running event finish, then stay paused
    }
    else
    {
        m_stepReq = true;
    }
}

void AnimationController::skipAll()
{
    if (!m_busy)
    {
        return;
    }
    m_skipping = true;
    while (m_busy)
    {
        if (!m_active)
        {
            beginCurrent();
        }
        completeCurrent();
    }
    m_skipping = false;
}

void AnimationController::startReplay()
{
    if (m_count == 0)
    {
        return;
    }
    if (m_hasStart)
    {
        m_vis->snapTo(m_start);   // also clears highlights
    }
    startPlayback();
}

void AnimationController::setSpeed(float s)
{
    if (s < 0.1f)
    {
        s = 0.1f;
    }
    if (s > 5.f)
    {
        s = 5.f;
    }
    m_speed = s;
}

// --------------------------------------------------------------------- state

bool AnimationController::busy() const
{
    return m_busy;
}

bool AnimationController::paused() const
{
    return m_paused;
}

float AnimationController::speed() const
{
    return m_speed;
}

int AnimationController::playedCount() const
{
    return m_index;
}

int AnimationController::totalCount() const
{
    return m_count;
}

float AnimationController::scrubProgress() const
{
    if (m_count == 0)
    {
        return 1.f;
    }
    return (float)m_index / (float)m_count;
}