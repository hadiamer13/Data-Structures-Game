// gui/widgets/Button.cpp
#include "Button.h"
#include "../Theme.h"

namespace
{

    const unsigned kMinTextSize = 12;   // label is size se chhota nahi hoga

    float minF(float a, float b)
    {
        return a < b ? a : b;
    }

    // Aik color channel ko t (0..1) ke hisaab se x se y tak mix karta hai.
    sf::Uint8 mixChannel(sf::Uint8 x, sf::Uint8 y, float t) {
        return static_cast<sf::Uint8>(x + (y - x) * t);
    }

    // Do colors ke darmiyan smooth transition ke liye.
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
        return sf::Color(mixChannel(a.r, b.r, t), mixChannel(a.g, b.g, t),
                         mixChannel(a.b, b.b, t), mixChannel(a.a, b.a, t));
    }

    float maxF(float a, float b)
    {
        return a > b ? a : b;
    }

} // namespace

// Button ko shuru karta hai: position, size, font aur label set karta hai.
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

// Label badalta hai aur layout dobara banata hai.
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

// Frame texture set karta hai. Texture na ho to frame band kar deta hai.
void Button::setFrame(const sf::Texture* frame) {
    m_hasFrame = (frame != nullptr && frame->getSize().x > 0 && frame->getSize().y > 0);
    if (m_hasFrame)
    {
        m_frame.setTexture(frame, Theme::FrameBorderPx);
    }
    else 
    {
        m_frame.setTexture(nullptr, 0);   // texture missing: saada button rakho
    }
    applyLayout();
}

// Label, box aur frame teeno ka size aur position aik hi rectangle se tay karta hai.
void Button::applyLayout() {
    // 1. Label ka base size lo, aur sirf max width set ho to chhota karo.
    m_text.setCharacterSize(m_textSize);
    if (m_maxW > 0.f)
    {
        while (m_text.getCharacterSize() > kMinTextSize &&
               m_text.getLocalBounds().width + 2.f * m_padX > m_maxW)
        {
            m_text.setCharacterSize(m_text.getCharacterSize() - 1);
        }
    }

    // 2. Chaurai = label + padding, minimum se kam nahi, max se zyada nahi.
    float w = maxF(m_minSize.x, m_text.getLocalBounds().width + 2.f * m_padX);
    if (m_maxW > 0.f)
    {
        w = minF(w, m_maxW);
    }
    m_size = sf::Vector2f(w, m_minSize.y);

    // 3. Box ko m_center par set karo.
    m_box.setSize(m_size);
    m_box.setOrigin(w / 2.f, m_size.y / 2.f);
    m_box.setPosition(m_center);

    // 4. Label ko bilkul center mein rakho.
    sf::FloatRect b = m_text.getLocalBounds();
    m_text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    m_text.setPosition(m_center);

    // 5. Frame bilkul box ke barabar ho taake padosi buttons par na chala jaye.
    m_frame.setRect(sf::FloatRect(m_center.x - w / 2.f, m_center.y - m_size.y / 2.f,
                                  w, m_size.y));
}

// Mouse events ko samajhta hai: hover, press aur click.
void Button::handleEvent(const sf::Event& e) {
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

// Hover ka smooth animation aur rang/outline ko har frame update karta hai.
void Button::update(float dt) {
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

    if (m_hasFrame) {
        // Frame ke saath fill dark rehti hai. Hover sirf frame ko roshan karta hai.
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

// Box, frame (agar ho) aur label ko window par draw karta hai.
void Button::draw(sf::RenderWindow& window) const {
    window.draw(m_box);
    if (m_hasFrame)
    {
        m_frame.draw(window);
    }
    window.draw(m_text);
}