// gui/widgets/Button.h
#pragma once
#include <SFML/Graphics.hpp>

// Reusable button. Knows NOTHING about screens or game logic.
// The owner registers a plain function pointer + context; the button
// calls it on click.
//
// Usage (in MainMenu.cpp):
//   static void onPlay(void* ctx) { ((MainMenu*)ctx)->goCampaign(); }
//   btn.setCallback(onPlay, this);
typedef void (*ButtonCallback)(void* context);

class Button {
public:
    // center = middle of the button in window pixels.
    // label is a plain C string (copied into sf::Text).
    void init(const char* label, const sf::Font& font,
        sf::Vector2f center, sf::Vector2f size);

    void setCallback(ButtonCallback cb, void* context) {
        m_onClick = cb;
        m_context = context;
    }
    void setLabel(const char* label);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    // States as plain ints (no enums).
    static const int StateIdle = 0;
    static const int StateHover = 1;
    static const int StatePressed = 2;

    void centerText();

    sf::RectangleShape m_box;
    sf::Text           m_text;          // created once, only setString on change
    int                m_state = StateIdle;
    float              m_hoverAmt = 0.f;  // 0..1, eased for smooth colour fade
    ButtonCallback     m_onClick = nullptr;
    void* m_context = nullptr;
};