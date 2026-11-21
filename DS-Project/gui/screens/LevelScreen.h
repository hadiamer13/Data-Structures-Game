// gui/screens/LevelScreen.h
// Real level screen: GameSession + AnimationController + Renderer.
// Playable missions: Act 1 (1.1 to 1.5), Act 2 (2.1, 2.2, 2.3, 2.5), Act 3 (3.1 to 3.5).
// Not yet: 2.4 (range), Act 4 (HeapLevel screen).
// Flow: input -> PlayerAction -> GameSession::apply -> events -> animation -> Result screen.
//
// Input is chosen per mission (see InputMode):
//   Key      BST missions: type / click a key, press Search, Insert or a delete-case button
//   Sequence 1.1: click rooms in traversal order (auto-submits at the last room)
//   Numbers  1.3: type height and diameter, press Submit
//   Click    1.4, 3.1, 3.5: every room click is sent as ClickNode
//   Pivot    3.2, 3.3: drag the pivot onto one of its children, or click it and press a rotation button
//   Buttons  1.5 (Mirror), 3.4 (predict the rotation)
//   Place    1.2: drag a token from the shelf onto a placed room (left or right side of it)
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

namespace InputMode
{
    inline const int Key = 0;
    inline const int Sequence = 1;
    inline const int Numbers = 2;
    inline const int Click = 3;
    inline const int Pivot = 4;
    inline const int Buttons = 5;
    inline const int Place = 6;
}

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

    static const int kMaxAct = 6;

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

    static int modeFor(int mission);

    void buildLayout();
    bool loadLevel();
    void buildActionButtons();
    void setMessage(const std::string& msg);
    void refreshHud(const HudState& h);
    void refreshStatus();
    void refreshPrompt();
    void applyMissionMarks();

    void doAction(int which);
    void submit(const PlayerAction& act);
    void handleTokenDrag(const sf::Event& e);
    void dropToken(int token, int mx, int my);
    void dropPivot(int pivot, int mx, int my);
    int tokenAt(int mx, int my) const;
    void drawTokens(sf::RenderWindow& window);
    void drawDragged(sf::RenderWindow& window);
    void drawPivotDrag(sf::RenderWindow& window);
    void drawSlotMarkers(sf::RenderWindow& window);
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
    bool fieldsActive() const;
    std::string keyOf(int id) const;

    AppContext& m_ctx;
    int m_index;
    bool m_ok;
    int m_mode;
    std::string m_error;
    LevelDef m_def;
    GameSession m_session;

    HudState m_lastHud;
    int m_shownAlarm;
    float m_endTimer;
    int m_selected;
    IntArray m_clicks; // 1.1: rooms picked so far, in order
    IntArray m_tokens; // 1.2: token values still on the shelf
    int m_dragToken;   // 1.2: shelf index being dragged, -1 = none
    int m_dragX;
    int m_dragY;
    int m_dragPivot; // 3.2 / 3.3: pivot room being dragged, kNull = none

    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_canvasBox;
    sf::RectangleShape m_bar;
    sf::Text m_hint;
    sf::Text m_msg;
    sf::Text m_prompt;
    sf::Text m_targetText;
    sf::Text m_notice;
    sf::CircleShape m_tokenCircle; // 1.2 shelf and dragged token
    sf::CircleShape m_slotCircle;  // 1.2 open-slot marker
    sf::Text m_tokenText;
    sf::Text m_slotText;
    sf::Text m_rootText;
    sf::RectangleShape m_rootBox;

    Renderer m_renderer;
    AnimationController m_anim; // must come after m_renderer
    Hud m_hud;
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    NumberField m_field;
    NumberField m_field2; // 1.3: diameter
    Button m_act[kMaxAct];
    ActCtx m_actCtx[kMaxAct];
    int m_actCount;

    int m_pressX;
    int m_pressY;
    bool m_pressing;
};