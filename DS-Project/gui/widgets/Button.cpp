// gui/widgets/Button.cpp
#include "Button.h"
#include "../Theme.h"

namespace
{

    const unsigned kMinTextSize = 12; // labels never shrink below this

    float minF(float a, float b)
    {
        return a < b ? a : b;
    }

    float maxF(float a, float b)
    {
        return a > b ? a : b;
    }

    sf::Uint8 mixChannel(sf::Uint8 x, sf::Uint8 y, float t)
    {
        return static_cast<sf::Uint8>(x + (y - x) * t);
    }

    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t)
    {
        return sf::Color(mixChannel(a.r, b.r, t), mixChannel(a.g, b.g, t),
                         mixChannel(a.b, b.b, t), mixChannel(a.a, b.a, t));
    }

} // namespace

void Button::init(const char* label, const sf::Font& font,
                  sf::Vector2f center, sf::Vector2f size)
{
    m_center = center;
    m_minSize = size;
    m_size = size;
    m_textSize = Theme::ButtonTextSize;

    m_box.setFillColor(Theme::BtnIdle);
    m_box.setOutlineColor(Theme::BtnOutline);
    m_box.setOutlineThickness(2.f);

    m_text.setFont(font); // SFML 2.x API
    m_text.setFillColor(Theme::TextColor);
    setLabel(label);
}

void Button::setLabel(const char* label)
{
    m_text.setString(label);
    applyLayout();
}

void Button::setPadding(float padX)
{
    m_padX = padX;
    applyLayout();
}

void Button::setMaxWidth(float maxW)
{
    m_maxW = (maxW > 0.f) ? maxW : 0.f;
    applyLayout();
}

void Button::setTextSize(unsigned size)
{
    m_textSize = size;
    applyLayout();
}

void Button::setCenter(sf::Vector2f c)
{
    m_center = c;
    applyLayout();
}

void Button::setFrame(const sf::Texture* frame)
{
    m_hasFrame = (frame != nullptr && frame->getSize().x > 0 && frame->getSize().y > 0);
    if (m_hasFrame)
    {
        m_frame.setTexture(frame, Theme::FrameBorderPx);
    }
    else
    {
        m_frame.setTexture(nullptr, 0); // texture missing: keep the plain button
    }
    applyLayout();
}

// Sizes the label, then the box, then the frame. All three use the same rectangle.
void Button::applyLayout()
{
    // 1. Start from the base label size and shrink it only if a max width is set.
    m_text.setCharacterSize(m_textSize);
    if (m_maxW > 0.f)
    {
        while (m_text.getCharacterSize() > kMinTextSize &&
               m_text.getLocalBounds().width + 2.f * m_padX > m_maxW)
        {
            m_text.setCharacterSize(m_text.getCharacterSize() - 1);
        }
    }

    // 2. Width = label + padding, not below the minimum, not above the max.
    float w = maxF(m_minSize.x, m_text.getLocalBounds().width + 2.f * m_padX);
    if (m_maxW > 0.f)
    {
        w = minF(w, m_maxW);
    }
    m_size = sf::Vector2f(w, m_minSize.y);

    // 3. Box, centred on m_center.
    m_box.setSize(m_size);
    m_box.setOrigin(w / 2.f, m_size.y / 2.f);
    m_box.setPosition(m_center);

    // 4. Label, centred.
    sf::FloatRect b = m_text.getLocalBounds();
    m_text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    m_text.setPosition(m_center);

    // 5. Frame covers exactly the box, so it never spills into neighbours.
    m_frame.setRect(sf::FloatRect(m_center.x - w / 2.f, m_center.y - m_size.y / 2.f,
                                  w, m_size.y));
}

void Button::handleEvent(const sf::Event& e)
{
    sf::FloatRect bounds = m_box.getGlobalBounds();

    if (e.type == sf::Event::MouseMoved)
    {
        if (m_state != StatePressed)
        {
            bool in = bounds.contains(static_cast<float>(e.mouseMove.x),
                                      static_cast<float>(e.mouseMove.y));
            m_state = in ? StateHover : StateIdle;
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed)
    {
        if (e.mouseButton.button == sf::Mouse::Left &&
            bounds.contains(static_cast<float>(e.mouseButton.x),
                            static_cast<float>(e.mouseButton.y)))
        {
            m_state = StatePressed;
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased)
    {
        if (e.mouseButton.button == sf::Mouse::Left &&
            m_state == StatePressed)
        {
            bool still = bounds.contains(static_cast<float>(e.mouseButton.x),
                                         static_cast<float>(e.mouseButton.y));
            m_state = still ? StateHover : StateIdle;
            if (still && m_onClick)
            {
                m_onClick(m_context);
            }
        }
    }
}

void Button::update(float dt)
{
    float target = (m_state == StateIdle) ? 0.f : 1.f;
    float step = 8.f * dt;
    if (m_hoverAmt < target)
    {
        m_hoverAmt = minF(target, m_hoverAmt + step);
    }
    else
    {
        m_hoverAmt = maxF(target, m_hoverAmt - step);
    }

    if (m_hasFrame)
    {
        // Framed: the fill stays dark. Hover only brightens the frame.
        m_box.setFillColor(sf::Color::Transparent);
        m_box.setOutlineThickness(0.f);
        m_frame.setColor(lerpColor(Theme::BtnOutline, Theme::NodeGlow, m_hoverAmt));
    }
    else
    {
        sf::Color fill = lerpColor(Theme::BtnIdle, Theme::BtnHover, m_hoverAmt);
        if (m_state == StatePressed)
        {
            fill = Theme::BtnPressed;
        }
        m_box.setFillColor(fill);
        m_box.setOutlineThickness(2.f + 2.f * m_hoverAmt);
    }
}

void Button::draw(sf::RenderWindow& window) const
{
    window.draw(m_box);
    if (m_hasFrame)
    {
        m_frame.draw(window);
    }
    window.draw(m_text);
}