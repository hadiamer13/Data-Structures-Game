// gui/widgets/NumberField.h
// Sirf digits wala text field (aik leading minus allowed). Click par focus, Backspace se delete,
// Enter par callback chalta hai. Screen ya model ke bare mein kuch nahi janta.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

typedef void (*NumberFieldCallback)(void* context);

class NumberField
{
  public:
    NumberField();

    // x, y = box ka top-left. Label box ke upar draw hota hai.
    void init(const std::string& label, const sf::Font& font, float x, float y, float w, float h);
    void setCallback(NumberFieldCallback cb, void* context); // Enter pressed

    void setValue(int v);
    void clear();
    bool hasValue() const; // false when empty or just "-"
    int value() const;     // 0 when !hasValue()
    bool focused() const;
    void setFocused(bool f);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    // Text badalne par caret ki position aur sprite dobara set karta hai.
    void refreshText();

    static const int kMaxChars = 7;

    float m_x;
    float m_y;
    float m_w;
    float m_h;
    std::string m_text;
    bool m_focused;
    float m_time;              // caret blink ke liye timer

    NumberFieldCallback m_cb;
    void* m_cbContext;

    sf::RectangleShape m_box;
    sf::RectangleShape m_caret;
    sf::Text m_label;
    sf::Text m_shown;
};