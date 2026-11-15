// gui/screens/LevelScreen.h
// Level shell: canvas (Renderer), HUD, toolbar, event log, tooltip.
// Playback is done by the AnimationController. The scenario itself is still a
// FAKE one (hand-made snapshots + scripted events) until GameSession exists.
//
// INTEGRATION: everything marked "FAKE DRIVER" is replaced by
//   GameSession::apply(PlayerAction) -> ActionResult{events, hud}
//   -> m_anim.beginBatch / enqueue / endBatch -> Renderer.
// The widgets, layout, input routing, controller wiring and Esc handling stay.
#pragma once
#include <string>
#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../AnimationController.h"
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
    static void onAnimEvent(void* context, const Event& e, int index, int total);
    static void onAnimDone(void* context);

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

    // ---- animation callbacks ----
    void animEvent(const Event& e);
    void animDone();

    // ---- HUD ----
    void refreshHud();

    // ---- FAKE DRIVER (INTEGRATION: delete) ----
    void loadFakeScenario();
    void addEvent(const Event& e);
    void startBatch();

    AppContext& m_ctx;
    int m_index;                 // level index 0..19 (act * 5 + level)

    // visuals
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_canvasBox;
    sf::Text m_canvasHint;

    Renderer m_renderer;
    AnimationController m_anim;  // must come after m_renderer (it points at it)
    Hud m_hud;
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    // input state
    int m_pressX;
    int m_pressY;
    bool m_pressing;

    // ---- fake scenario state ----
    static const int kMaxQueue = 64;
    Event m_queue[kMaxQueue];
    int m_qCount;
    bool m_done;

    ViewSnapshot m_start;
    ViewSnapshot m_final;
    int m_targetId;

    int m_alarm;
    int m_alarmCap;
    int m_optimal;
};