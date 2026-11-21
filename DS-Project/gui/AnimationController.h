// gui/AnimationController.h
// Plays one batch of events (one player action) through an IEventVisualizer.
// Timing, queue, pause, single-step, skip, replay and speed live here.
//
// Usage per player action (the batch must be built in this order):
//   ctrl.beginBatch(snapshotBefore);
//   ctrl.enqueue(event, snapshotAfterThisEvent or nullptr);   // per event
//   ctrl.endBatch(snapshotAfterWholeAction);                  // starts playing
//
// Snapshot timing per event (so structure and effects never fight):
//   INSERT, ROTATE : animateTo(after) BEFORE beginEvent (new node must exist)
//   UNLINK         : animateTo(after) right AFTER beginEvent (node fades while playing)
//   everything else: animateTo(after) AFTER endEvent (swaps finish their own travel first)
#pragma once
#include "IEventVisualizer.h"

typedef void (*AnimEventCallback)(void* context, const Event& e, int index, int total);
typedef void (*AnimDoneCallback)(void* context);

struct AnimItem
{
    Event ev;
    ViewSnapshot* after; // owned by the controller, may be nullptr

    AnimItem() : after(nullptr)
    {
    }
};

class AnimationController
{
  public:
    explicit AnimationController(IEventVisualizer* visualizer);
    ~AnimationController();

    // Called when an event starts playing (log line, alarm update) and when the batch ends.
    void setCallbacks(AnimEventCallback onEvent, AnimDoneCallback onDone, void* context);

    // Batch building
    void beginBatch(const ViewSnapshot& before);
    void enqueue(const Event& e, const ViewSnapshot* afterEvent);
    void endBatch(const ViewSnapshot& afterAll); // starts playback

    // Frame
    void update(float dt);

    // Controls
    void play();
    void pause();
    void togglePause();
    void stepOne();         // plays exactly one event, then stays paused
    void skipAll();         // finish instantly, hard-sync to the end snapshot
    void startReplay();     // snap to the start snapshot and play again
    void setSpeed(float s); // 0.1 .. 5 (fed from Toolbar::speed())
    void clear();           // drop the batch and stop

    // State
    bool busy() const; // true while events remain (LevelScreen locks input)
    bool paused() const;
    float speed() const;
    int playedCount() const;
    int totalCount() const;
    float scrubProgress() const; // 0..1, for a future scrubber bar

    static float eventDuration(int eventType); // seconds at speed 1

  private:
    AnimationController(const AnimationController&);
    AnimationController& operator=(const AnimationController&);

    void startPlayback();
    void beginCurrent();
    void completeCurrent();
    void finishBatch();
    void freeItems();

    IEventVisualizer* m_vis;

    AnimItem* m_items;
    int m_count;
    int m_cap;

    ViewSnapshot m_start;
    ViewSnapshot m_end;
    bool m_hasStart;
    bool m_hasEnd;

    int m_index;
    float m_t;
    float m_speed;
    bool m_busy;
    bool m_paused;
    bool m_active;   // an event is animating
    bool m_stepping; // single step in progress
    bool m_stepReq;
    bool m_skipping;

    AnimEventCallback m_onEvent;
    AnimDoneCallback m_onDone;
    void* m_cbContext;
};