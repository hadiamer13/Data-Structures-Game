// gui/screens/LevelScreen.h
// Level shell: canvas (Renderer), HUD, toolbar, event log, tooltip.
// Today it is driven by a FAKE driver (hand-made snapshots + scripted events)
// so every widget and the Renderer can be tested before GameSession and
// AnimationController exist.
//
// INTEGRATION: everything marked "FAKE DRIVER" is replaced by
//   GameSession::apply(PlayerAction) -> ActionResult{events, hud}
//   -> AnimationController -> Renderer (IEventVisualizer).
// The widgets, layout, input routing and Esc handling stay as they are.
#pragma once
#include <string>
#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../widgets/Hud.h"
#include "../widgets/Toolbar.h"
#include "../widgets/EventLogPanel.h"
#include "../widgets/Tooltip.h"

class LevelScreen : public Screen
{
public:
    explicit LevelScreen(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static toolbar callback (public so it can reach it).
    void onToolbar(int action);

private:
    LevelScreen(const LevelScreen&);
    LevelScreen& operator=(const LevelScreen&);

    static void onToolbarClick(void* context, int action);

    // ---- layout / input ----
    void buildLayout();
    void updateTooltip(int mx, int my);
    void onCanvasClick(int mx, int my);

    // ---- toolbar actions ----
    void doUndo();
    void doReplay();
    void doReset();
    void doHint();
    void togglePlay();
    void stepOnce();

    // ---- HUD ----
    void refreshHud();

    // ---- FAKE DRIVER (INTEGRATION: delete) ----
    void loadFakeScenario();
    void addEvent(const Event& e);
    void startBatch();
    void advanceDriver(float dt);
    void beginCurrent();
    void finishBatch();

    AppContext& m_ctx;
    int m_index;                 // level index 0..19 (act * 5 + level)

    // visuals
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_canvasBox;
    sf::Text m_canvasHint;

    Renderer m_renderer;
    Hud m_hud;
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    // input state
    int m_pressX;
    int m_pressY;
    bool m_pressing;

    // ---- fake driver state ----
    static const int kMaxQueue = 64;
    Event m_queue[kMaxQueue];
    int m_qCount;
    int m_qIndex;
    bool m_busy;       // a batch is playing (input locked)
    bool m_paused;
    bool m_evActive;   // an event is currently animating
    bool m_stepping;   // single-step requested
    bool m_stepReq;
    bool m_done;
    float m_evT;       // seconds into the current event
    float m_speed;

    ViewSnapshot m_start;
    ViewSnapshot m_final;
    int m_targetId;

    int m_alarm;
    int m_alarmCap;
    int m_optimal;
};
