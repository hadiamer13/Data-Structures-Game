// gui/widgets/Button.h
// Reusable button. Isay screen ya game logic ke bare mein kuch pata nahi hota.
// Owner ek plain function pointer aur context register karta hai,
// button click par woh function call karta hai.
#pragma once
#include <SFML/Graphics.hpp>
#include "NineSlice.h"

typedef void (*ButtonCallback)(void* context);

//  button class
class Button {
public:
    // center button ka darmiyani nuqta hai, window pixels mein.
    // size.x kam se kam chaurai hai, button label ke hisaab se barh sakta hai.
    void init(const char* label, const sf::Font& font,
              sf::Vector2f center, sf::Vector2f size);

    // Click par chalne wala function aur uska context set karta hai.
    void setCallback(ButtonCallback cb, void* context) {
        m_onClick = cb;
        m_context = context;
    }

    void setLabel(const char* label);

    // Optional frame texture, nine slice ke taur par draw hoti hai taake distort na ho.
    void setFrame(const sf::Texture* frame);

    // Label aur button ke har side ke beech horizontal jagah.
    void setPadding(float padX);

    // Chaurai ki hard limit. Agar label fit na ho to pehle chhota hota hai.
    // 0 ka matlab koi limit nahi. Fixed-width row mein istemal karo.
    void setMaxWidth(float maxW);

    // Current width. This is the real width on screen (frame included).
    float width() const
    {
        return m_size.x;
    }

    // Centre ko move karta hai aur dobara layout karta hai.
    void setCenter(sf::Vector2f c);

    // Base label size. Lamba label maxWidth set hone par chhota ho sakta hai.
    void setTextSize(unsigned size);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

  private:
    static const int StateIdle = 0;
    static const int StateHover = 1;
    static const int StatePressed = 2;

    void applyLayout(); // measures the label, sets width, positions box/text/frame

    sf::RectangleShape m_box;
    sf::Text m_text;
    NineSlice m_frame;
    bool m_hasFrame = false;
    sf::Vector2f m_center;
    sf::Vector2f m_minSize;
    sf::Vector2f m_size;
    unsigned m_textSize = 0; // base label size, set in init()
    float m_padX = 36.f;
    float m_maxW = 0.f; // 0 = no limit
    int m_state = StateIdle;
    float m_hoverAmt = 0.f;
    ButtonCallback m_onClick = nullptr;
    void* m_context = nullptr;
};