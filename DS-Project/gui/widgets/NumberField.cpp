// gui/widgets/NumberField.cpp
#include "NumberField.h"
#include "../Theme.h"

NumberField::NumberField()
    : m_x(0.f), m_y(0.f), m_w(100.f), m_h(30.f), m_focused(false), m_time(0.f),
    m_cb(nullptr), m_cbContext(nullptr)
{
}

void NumberField::init(const std::string& label, const sf::Font& font, float x, float y, float w, float h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    m_box.setPosition(x, y);
    m_box.setSize(sf::Vector2f(w, h));
    m_box.setFillColor(Theme::BtnIdle);
    m_box.setOutlineColor(Theme::BtnOutline);
    m_box.setOutlineThickness(2.f);

    m_label.setFont(font);
    m_label.setCharacterSize(14);
    m_label.setFillColor(Theme::NodeGlow);
    m_label.setString(label);
    m_label.setPosition(x, y - 20.f);

    m_shown.setFont(font);
    m_shown.setCharacterSize(20);
    m_shown.setFillColor(Theme::TextColor);
    m_shown.setPosition(x + 8.f, y + h * 0.5f - 15.f);

    m_caret.setSize(sf::Vector2f(2.f, h - 12.f));
    m_caret.setFillColor(Theme::TextColor);

    refreshText();
}

void NumberField::setCallback(NumberFieldCallback cb, void* context)
{
    m_cb = cb;
    m_cbContext = context;
}

void NumberField::setValue(int v)
{
    m_text = std::to_string(v);
    refreshText();
}

void NumberField::clear()
{
    m_text.clear();
    refreshText();
}

bool NumberField::hasValue() const
{
    return !m_text.empty() && m_text != "-";
}

int NumberField::value() const
{
    if (!hasValue())
    {
        return 0;
    }
    int v = 0;
    bool neg = false;
    for (size_t i = 0; i < m_text.size(); ++i)
    {
        char c = m_text[i];
        if (c == '-')
        {
            neg = true;
        }
        else
        {
            v = v * 10 + (c - '0');
        }
    }
    return neg ? -v : v;
}

bool NumberField::focused() const
{
    return m_focused;
}

void NumberField::setFocused(bool f)
{
    m_focused = f;
}

void NumberField::refreshText()
{
    m_shown.setString(m_text);
    sf::FloatRect b = m_shown.getLocalBounds();
    m_caret.setPosition(m_x + 8.f + b.left + b.width + 3.f, m_y + 6.f);
}

void NumberField::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        m_focused = m_box.getGlobalBounds().contains((float)e.mouseButton.x, (float)e.mouseButton.y);
        m_time = 0.f;
    }
    else if (m_focused && e.type == sf::Event::TextEntered)
    {
        unsigned int u = e.text.unicode;
        if (u >= 48 && u <= 57)
        {
            if ((int)m_text.size() < kMaxChars)
            {
                m_text += (char)u;
            }
        }
        else if (u == 45)
        {
            if (m_text.empty())
            {
                m_text = "-";
            }
        }
        else if (u == 8)
        {
            if (!m_text.empty())
            {
                m_text.erase(m_text.size() - 1, 1);
            }
        }
        refreshText();
    }
    else if (m_focused && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Return)
    {
        if (m_cb)
        {
            m_cb(m_cbContext);
        }
    }
}

void NumberField::update(float dt)
{
    m_time += dt;
    // Not colour alone: focus = thick outline + blinking caret.
    m_box.setOutlineThickness(m_focused ? 4.f : 2.f);
    m_box.setFillColor(m_focused ? Theme::BtnHover : Theme::BtnIdle);
}

void NumberField::draw(sf::RenderWindow& window) const
{
    window.draw(m_box);
    window.draw(m_label);
    window.draw(m_shown);
    if (m_focused && ((int)(m_time * 2.f) % 2 == 0))
    {
        window.draw(m_caret);
    }
}