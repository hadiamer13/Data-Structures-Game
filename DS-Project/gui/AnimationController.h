// gui/AnimationController.h
// Aik player action ka batch (events ki list) IEventVisualizer ke zariye chalata hai.
// Timing, queue, pause, single step, skip, replay aur speed sab yahin hain.
//
// Istemal ka tareeqa (har player action ke liye, isi tarteeb se):
//   ctrl.beginBatch(snapshotBefore);
//   ctrl.enqueue(event, snapshotAfterThisEvent ya nullptr);   /har event ke liye
//   ctrl.endBatch(snapshotAfterWholeAction);                   playback shuru
//
// Har event ke liye snapshot ka waqt (taake structure aur effects aapas mein na ladein):
//   INSERT, ROTATE: animateTo(after) BEFORE beginEvent (naya node pehle maujood hona chahiye)
//   UNLINK: animateTo(after) beginEvent ke FORAN BAAD (node chalte hue fade hota hai)
//   baaqi sab: animateTo(after) endEvent KE BAAD (swap apna safar khatam kare)
#pragma once
#include "IEventVisualizer.h"

typedef void (*AnimEventCallback)(void* context, const Event& e, int index, int total);
typedef void (*AnimDoneCallback)(void* context);

struct AnimItem
{
    Event ev;
    ViewSnapshot* after;   // controller ki ownership mein, nullptr ho sakta hai

    AnimItem() : after(nullptr)
    {
    }
};

class AnimationController
{
public:
    explicit AnimationController(IEventVisualizer* visualizer);
    ~AnimationController();

    // Jab koi event chalna shuru ho (log line, alarm update) aur jab batch khatam ho, tab yeh callbacks chalte hain.
    void setCallbacks(AnimEventCallback onEvent, AnimDoneCallback onDone, void* context);

    // Batch banana
    void beginBatch(const ViewSnapshot& before);
    void enqueue(const Event& e, const ViewSnapshot* afterEvent);
    void endBatch(const ViewSnapshot& afterAll);   // playback shuru karta hai

    
    void update(float dt);

    // Controls
    void play();
    void pause();
    void togglePause();
    void stepOne();          // exactly aik event chalata hai, phir pause par rehta hai
    void skipAll();          // sab fauran khatam, end snapshot par hard sync
    void startReplay();      // start snapshot par wapas jaake dobara chalata hai
    void setSpeed(float s);  // 0.1 .. 5   Toolbar::speed() se aata hai
    void clear();            // batch khatam karke rok deta hai

    // State
    bool busy() const;       // jab tak events baaki hain, true (LevelScreen input lock karti hai)
    bool paused() const;
    float speed() const;
    int playedCount() const;
    int totalCount() const;
    float scrubProgress() const;   // 0..1, future scrubber bar ke liye

    static float eventDuration(int eventType);   // speed 1 par seconds mein

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
    bool m_active;     // koi event chal raha hai
    bool m_stepping;   // single step chal raha hai
    bool m_stepReq;
    bool m_skipping;

    AnimEventCallback m_onEvent;
    AnimDoneCallback m_onDone;
    void* m_cbContext;
};