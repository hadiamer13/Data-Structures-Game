#pragma once

// IEventVisualizer: AnimationController jis interface se baat karta hai.
// Renderer ise implement karta hai. Timing controller ke paas hai; visualizer sirf react karta hai:
// beginEvent aik dafa, progress(t, 0..1) har frame, endEvent aik dafa.
// snapTo() hard sync hai (tween nahi), Undo / Reset / Load ke baad istemal hota hai.

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