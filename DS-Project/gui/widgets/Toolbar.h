// gui/widgets/Toolbar.h
// Bottom bar: Undo, Replay, Reset, Hint buttons + animation speed slider.
// Reports clicks through one function pointer: callback(context, action).
#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"

namespace ToolbarAction
{
    inline const int Undo = 0;
    inline const int Replay = 1;
    inline const int Reset = 2;
    inline const int Hint = 3;
}

typedef void (*ToolbarCallback)(void* context, int action);

class Toolbar;

// One small context per button so a single static function serves all four.
struct ToolbarButtonCtx
{
    Toolbar* owner;
    int action;
};

class Toolbar
{
  public:
    Toolbar();

    void init(const sf::Font& font, float x, float y, float w, float h);
    void setCallback(ToolbarCallback cb, void* context);

    float speed() const; // 0.25 .. 3.0, default 1.0

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    // Called by the static button callback (public so it can reach it).
    void fire(int action);

  private:
    static void onButton(void* context);

    void setSpeedFromX(float x);
    void refreshSpeedLabel();
    void layoutKnob();

    static const int kButtons = 4;

    sf::RectangleShape m_bg;
    Button m_btn[kButtons];
    ToolbarButtonCtx m_btnCtx[kButtons];

    // slider
    sf::Text m_speedTitle;
    sf::Text m_speedValue;
    sf::RectangleShape m_track;
    sf::RectangleShape m_knob;
    float m_trackX0;
    float m_trackX1;
    float m_trackY;
    float m_speed;
    bool m_dragging;

    ToolbarCallback m_cb;
    void* m_cbContext;
};
