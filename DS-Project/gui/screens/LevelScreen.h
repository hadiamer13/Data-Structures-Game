// gui/screens/LevelScreen.h
// Real level screen: GameSession + AnimationController + Renderer.
// Playable missions today: 2.1 search, 2.2 insert, 2.3 delete, 2.5 boss (BST).
// Other missions show a "not implemented yet" panel.
// Flow: input -> PlayerAction -> GameSession::apply -> events -> animation -> Result screen.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../AnimationController.h"
#include "../widgets/Button.h"
#include "../widgets/NumberField.h"
#include "../widgets/Hud.h"
#include "../widgets/Toolbar.h"
#include "../widgets/EventLogPanel.h"
#include "../widgets/Tooltip.h"
#include "../../game/GameSession.h"

class LevelScreen : public Screen
{
public:
    explicit LevelScreen(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Called by the static callbacks (public so they can reach them).
    void onToolbar(int action);
    void onAction(int which);
    void onFieldEnter();

private:
    LevelScreen(const LevelScreen&);
    LevelScreen& operator=(const LevelScreen&);

    struct ActCtx
    {
        LevelScreen* owner;
        int which;
    };

    static void onToolbarClick(void* context, int action);
    static void onActionClick(void* context);
    static void onFieldClick(void* context);
    static void onAnimEvent(void* context, const Event& e, int index, int total);
    static void onAnimDone(void* context);

    void buildLayout();
    bool loadLevel();
    void buildActionButtons();
    void setMessage(const std::string& msg);
    void refreshHud(const HudState& h);
    void refreshStatus();

    void doAction(int which);
    void doUndo();
    void doReset();
    void doHint();
    void play(const ViewSnapshot& before, const ActionResult& r);
    void goToResult();

    void animEvent(const Event& e);
    void animDone();
    void updateTooltip(int mx, int my);
    void onCanvasClick(int mx, int my);
    bool typing() const;

    AppContext& m_ctx;
    int m_index;
    bool m_ok;
    std::string m_error;
    LevelDef m_def;
    GameSession m_session;

    HudState m_lastHud;
    int m_shownAlarm;
    float m_endTimer;
    int m_selected;

    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_canvasBox;
    sf::RectangleShape m_bar;
    sf::Text m_hint;
    sf::Text m_msg;
    sf::Text m_targetText;
    sf::Text m_notice;

    Renderer m_renderer;
    AnimationController m_anim;   // must come after m_renderer
    Hud m_hud;
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    NumberField m_field;
    Button m_act[3];
    ActCtx m_actCtx[3];
    int m_actCount;

    int m_pressX;
    int m_pressY;
    bool m_pressing;
};