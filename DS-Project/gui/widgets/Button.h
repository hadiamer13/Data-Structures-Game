// gui/widgets/Button.h
#pragma once
#include <SFML/Graphics.hpp>
#include "NineSlice.h"

// Reusable button. Knows NOTHING about screens or game logic.
// The owner registers a plain function pointer + context; the button
// calls it on click.
typedef void (*ButtonCallback)(void* context);

class Button
{
  public:
    // center = middle of the button in window pixels.
    // size.x is the MINIMUM width: the button widens to fit its label.
    void init(const char* label, const sf::Font& font,
              sf::Vector2f center, sf::Vector2f size);

    void setCallback(ButtonCallback cb, void* context)
    {
        m_onClick = cb;
        m_context = context;
    }

    void setLabel(const char* label);

    // Optional frame texture, drawn as a nine-slice so it never distorts.
    // The frame always covers exactly the button's own rectangle.
    void setFrame(const sf::Texture* frame);

    // Horizontal space between the label and each side of the button.
    void setPadding(float padX);

    // Hard limit on the width. A label that does not fit shrinks first.
    // 0 = no limit. Use this where a button sits in a fixed-width row.
    void setMaxWidth(float maxW);

    // Current width. This is the real width on screen (frame included).
    float width() const
    {
        return m_size.x;
    }

    // Moves the button's centre and re-lays it out.
    void setCenter(sf::Vector2f c);

    // Base label size. Long labels may shrink below this when maxWidth is set.
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