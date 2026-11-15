// gui/widgets/NumberField.h
// Digits-only text field (optional leading minus). Click to focus, Backspace deletes,
// Enter fires the callback. Knows nothing about screens or the model.
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

typedef void (*NumberFieldCallback)(void* context);

class NumberField
{
public:
    NumberField();

    // x, y = top-left of the box. The label is drawn above it.
    void init(const std::string& label, const sf::Font& font, float x, float y, float w, float h);
    void setCallback(NumberFieldCallback cb, void* context);   // Enter pressed

    void setValue(int v);
    void clear();
    bool hasValue() const;     // false when empty or just "-"
    int value() const;         // 0 when !hasValue()
    bool focused() const;
    void setFocused(bool f);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    void refreshText();

    static const int kMaxChars = 7;

    float m_x;
    float m_y;
    float m_w;
    float m_h;
    std::string m_text;
    bool m_focused;
    float m_time;

    NumberFieldCallback m_cb;
    void* m_cbContext;

    sf::RectangleShape m_box;
    sf::RectangleShape m_caret;
    sf::Text m_label;
    sf::Text m_shown;
};