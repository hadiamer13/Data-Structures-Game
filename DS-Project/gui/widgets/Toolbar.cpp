// gui/widgets/Toolbar.cpp
#include "Toolbar.h"
#include "../Theme.h"
#include <cstdio>

namespace
{
    const float MinSpeed = 0.25f;
    const float MaxSpeed = 3.0f;
}

Toolbar::Toolbar()
    : m_trackX0(0.f), m_trackX1(0.f), m_trackY(0.f),
    m_speed(1.f), m_dragging(false), m_cb(nullptr), m_cbContext(nullptr)
{
    for (int i = 0; i < kButtons; ++i)
    {
        m_btnCtx[i].owner = this;
        m_btnCtx[i].action = i;
    }
}

void Toolbar::init(const sf::Font& font, float x, float y, float w, float h)
{
    m_bg.setPosition(x, y);
    m_bg.setSize(sf::Vector2f(w, h));
    m_bg.setFillColor(Theme::TooltipBg);
    m_bg.setOutlineColor(Theme::BtnOutline);
    m_bg.setOutlineThickness(-2.f);

    const char* labels[kButtons] = { "Undo", "Replay", "Reset", "Hint" };
    const float bw = 120.f;
    const float bh = h - 12.f;
    const float cy = y + h * 0.5f;
    for (int i = 0; i < kButtons; ++i)
    {
        float cx = x + 16.f + bw * 0.5f + (float)i * (bw + 14.f);
        m_btn[i].init(labels[i], font, sf::Vector2f(cx, cy), sf::Vector2f(bw, bh));
        m_btn[i].setCallback(&Toolbar::onButton, &m_btnCtx[i]);
    }

    // speed slider on the right
    m_trackX1 = x + w - 100.f;
    m_trackX0 = m_trackX1 - 200.f;
    m_trackY = cy;

    m_speedTitle.setFont(font);
    m_speedTitle.setCharacterSize(16);
    m_speedTitle.setFillColor(Theme::TextColor);
    m_speedTitle.setString("SPEED");
    m_speedTitle.setPosition(m_trackX0 - 80.f, cy - 10.f);

    m_track.setSize(sf::Vector2f(m_trackX1 - m_trackX0, 6.f));
    m_track.setPosition(m_trackX0, cy - 3.f);
    m_track.setFillColor(sf::Color(60, 40, 46));
    m_track.setOutlineColor(Theme::NodeLine);
    m_track.setOutlineThickness(1.5f);

    m_knob.setSize(sf::Vector2f(14.f, 26.f));
    m_knob.setOrigin(7.f, 13.f);
    m_knob.setFillColor(Theme::BtnHover);
    m_knob.setOutlineColor(Theme::TextColor);
    m_knob.setOutlineThickness(2.f);

    m_speedValue.setFont(font);
    m_speedValue.setCharacterSize(16);
    m_speedValue.setFillColor(Theme::TextColor);
    m_speedValue.setPosition(m_trackX1 + 18.f, cy - 10.f);

    layoutKnob();
    refreshSpeedLabel();
}

void Toolbar::setCallback(ToolbarCallback cb, void* context)
{
    m_cb = cb;
    m_cbContext = context;
}

float Toolbar::speed() const
{
    return m_speed;
}

void Toolbar::onButton(void* context)
{
    ToolbarButtonCtx* c = static_cast<ToolbarButtonCtx*>(context);
    c->owner->fire(c->action);
}

void Toolbar::fire(int action)
{
    if (m_cb)
    {
        m_cb(m_cbContext, action);
    }
}

void Toolbar::layoutKnob()
{
    float t = (m_speed - MinSpeed) / (MaxSpeed - MinSpeed);
    m_knob.setPosition(m_trackX0 + t * (m_trackX1 - m_trackX0), m_trackY);
}

void Toolbar::refreshSpeedLabel()
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.2fx", m_speed);
    m_speedValue.setString(buf);
}

void Toolbar::setSpeedFromX(float x)
{
    float t = (x - m_trackX0) / (m_trackX1 - m_trackX0);
    if (t < 0.f)
    {
        t = 0.f;
    }
    if (t > 1.f)
    {
        t = 1.f;
    }
    m_speed = MinSpeed + t * (MaxSpeed - MinSpeed);
    layoutKnob();
    refreshSpeedLabel();
}

void Toolbar::handleEvent(const sf::Event& e)
{
    for (int i = 0; i < kButtons; ++i)
    {
        m_btn[i].handleEvent(e);
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float mx = (float)e.mouseButton.x;
        float my = (float)e.mouseButton.y;
        if (mx >= m_trackX0 - 10.f && mx <= m_trackX1 + 10.f
            && my >= m_trackY - 16.f && my <= m_trackY + 16.f)
        {
            m_dragging = true;
            setSpeedFromX(mx);
        }
    }
    else if (e.type == sf::Event::MouseMoved)
    {
        if (m_dragging)
        {
            setSpeedFromX((float)e.mouseMove.x);
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased)
    {
        m_dragging = false;
    }
}

void Toolbar::update(float dt)
{
    for (int i = 0; i < kButtons; ++i)
    {
        m_btn[i].update(dt);
    }
    // not colour alone: the knob grows while dragged
    m_knob.setOutlineThickness(m_dragging ? 4.f : 2.f);
}

void Toolbar::draw(sf::RenderWindow& window) const
{
    window.draw(m_bg);
    for (int i = 0; i < kButtons; ++i)
    {
        m_btn[i].draw(window);
    }
    window.draw(m_speedTitle);
    window.draw(m_track);
    window.draw(m_knob);
    window.draw(m_speedValue);
}
