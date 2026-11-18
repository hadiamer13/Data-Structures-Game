#pragma once

// IEventVisualizer: what the AnimationController talks to.
// Renderer implements it. The controller owns timing; the visualizer only
// reacts: beginEvent once, progress(t in 0..1) every frame, endEvent once.
// snapTo() is a hard sync (no tween), used after Undo / Reset / Load.

#include "../contracts/Event.h"
#include "../contracts/ViewSnapshot.h"

class IEventVisualizer
{
public:
    virtual ~IEventVisualizer() {}

    virtual void beginEvent(const Event& e) = 0;
    virtual void progress(const Event& e, float t) = 0;
    virtual void endEvent(const Event& e) = 0;
    virtual void snapTo(const ViewSnapshot& snap) = 0;
    virtual void animateTo(const ViewSnapshot& snap) = 0;  
};
