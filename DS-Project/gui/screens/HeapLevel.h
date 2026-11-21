#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../AnimationController.h"
#include "../widgets/Button.h"
#include "../widgets/Hud.h"
#include "../widgets/Toolbar.h"
#include "../widgets/EventLogPanel.h"
#include "../widgets/Tooltip.h"
#include "../../game/GameSession.h"

//  heap operations
namespace HeapOp
{
    inline const int Build = 0;
    inline const int Heapify = 1;
    inline const int Extract = 2;
    inline const int Clear = 3;
}


// Act 4 "Heap Fortress" screen, missions 4.1 se 4.5 tak.
// Heap ka tree view aur array view dono Renderer ek hi ViewSnapshot se banate hain,
// is liye woh hamesha synchronised rehte hain.
// Input: node ya array cell click karo. Aik select ho to uske adjacent (parent ya child) cell
// par click SwapHeap bhejta hai. Buttons: Build heap, Heapify selected, Extract max, Clear selection.
// Model decide karta hai ke har action valid hai ya nahi.
class HeapLevel : public Screen
{
  public:
    explicit HeapLevel(AppContext& ctx);

    void handleEvent(const sf::Event& e) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

  private:
    HeapLevel(const HeapLevel&);
    HeapLevel& operator=(const HeapLevel&);

    static const int kMaxOps = 4;

    struct OpCtx
    {
        HeapLevel* owner;
        int op;
    };

    static void onToolbarClick(void* context, int action);
    static void onOpClick(void* context);
    static void onAnimEvent(void* context, const Event& e, int index, int total);
    static void onAnimDone(void* context);

    void buildLayout();
    bool loadLevel();
    void buildOpButtons();
    void addOp(int op, const char* label);
    bool allowsBuild() const;
    bool allowsHeapify() const;
    bool allowsExtract() const;
    bool allowsSwap() const;

    void setMessage(const std::string& msg);
    void setSelection(int id);
    void refreshHud(const HudState& h);
    void refreshStatus();
    void refreshPrompt();
    void updateTooltip(int mx, int my);
    void onCanvasClick(int mx, int my);
    void onToolbar(int action);
    void onOp(int op);
    void submit(const PlayerAction& act);
    void play(const ViewSnapshot& before, const ActionResult& r);
    void doUndo();
    void doReset();
    void doHint();
    void animEvent(const Event& e);
    void animDone();
    void goToResult();

    AppContext& m_ctx;
    int m_index;
    bool m_ok;
    std::string m_error;
    LevelDef m_def;
    GameSession m_session;

    HudState m_lastHud;
    int m_selected;
    float m_endTimer;
    int m_pressX;
    int m_pressY;
    bool m_pressing;
    int m_opCount;
    float m_opNextX = 170.f;   // agle op button ka x, "Selected:" label ke baad

    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_canvasBox;
    sf::RectangleShape m_bar;
    sf::Text m_hint;
    sf::Text m_msg;
    sf::Text m_prompt;
    sf::Text m_selText;
    sf::Text m_notice;

    Renderer m_renderer;          // m_anim se pehle banna zaroori hai
    AnimationController m_anim;
    Hud m_hud;
    Toolbar m_toolbar;
    EventLogPanel m_log;
    Tooltip m_tooltip;

    Button m_op[kMaxOps];
    OpCtx m_opCtx[kMaxOps];
};