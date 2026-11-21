#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../AnimationController.h"
#include "../widgets/Button.h"
#include "../widgets/NumberField.h"
#include "../widgets/Toolbar.h"
#include "../widgets/EventLogPanel.h"
#include "../widgets/Tooltip.h"
#include "../../game/SandboxSession.h"

//  sandbox screen
// Upar tabs hain, bayein event log, beech mein canvas, daayein operation panel aur neeche toolbar.
// Sirf model se baat karta hai. Koi action khud reject nahi karta, model faisla karta hai.
class SandboxScreen : public Screen
{
public:
    explicit SandboxScreen(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // Static callbacks se call hote hain, isliye public hain.
    void onTab(int kind);
    void onOp(int op);
    void onToolbar(int action);
    void onFieldEnter();

private:
    SandboxScreen(const SandboxScreen&);
    SandboxScreen& operator=(const SandboxScreen&);

    static const int kTabs = 4;
    static const int kMaxOps = 16;

    struct TabCtx
    {
        SandboxScreen* owner;
        int kind;
    };
    struct OpCtx
    {
        SandboxScreen* owner;
        int op;
    };

    static void onTabClick(void* context);
    static void onOpClick(void* context);
    static void onToolbarClick(void* context, int action);
    static void onFieldClick(void* context);
    static void onAnimEvent(void* context, const Event& e, int index, int total);
    static void onAnimDone(void* context);

    void buildLayout();
    void loadKind(int kind, bool empty);
    void rebuildOps();
    void refreshTabs();
    void refreshInfo();
    void setMessage(const std::string& msg);
    void setValid(const std::string& text, bool ok);

    void doOp(int op);
    void doUndo();
    void doReset();
    void play(const SandboxResult& r);

    void updateTooltip(int mx, int my);
    void onCanvasClick(int mx, int my);
    bool typing() const;

    AppContext& m_ctx;
    SandboxSession m_session;
    int m_kind = TreeKind::BST;
    int m_seed = 1;
    int m_selected = kNull;
    int m_pendingFocus = kNull;

    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_tabBar;
    sf::RectangleShape m_tabUnder;
    sf::RectangleShape m_canvasBox;
    sf::RectangleShape m_panel;
    sf::Text m_title;
    sf::Text m_msg;
    sf::Text m_hint;
    sf::Text m_stepsText;
    sf::Text m_selText;
    sf::Text m_validText;

    Renderer m_renderer;
    AnimationController m_anim;   
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    Button m_tab[kTabs];
    TabCtx m_tabCtx[kTabs];

    Button m_op[kMaxOps];
    OpCtx m_opCtx[kMaxOps];
    int m_opCount = 0;

    NumberField m_fieldA;
    NumberField m_fieldB;

    int m_pressX = 0;
    int m_pressY = 0;
    bool m_pressing = false;
};