// gui/widgets/EventLogPanel.cpp
#include "EventLogPanel.h"
#include "../Theme.h"

namespace
{
    const float Pad = 10.f;
    const float HeaderH = 34.f;
    const float LineH = 18.f;
    const unsigned TextSize = 14;
}

EventLogPanel::EventLogPanel()
    : m_x(0.f), m_y(0.f), m_w(320.f), m_h(500.f),
      m_start(0), m_count(0), m_scroll(0), m_dirty(true)
{
}

void EventLogPanel::init(const sf::Font& font, float x, float y, float w, float h)
{
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    m_bg.setPosition(x, y);
    m_bg.setSize(sf::Vector2f(w, h));
    m_bg.setFillColor(Theme::TooltipBg);
    m_bg.setOutlineColor(Theme::BtnOutline);
    m_bg.setOutlineThickness(-2.f);

    m_header.setFont(font);
    m_header.setCharacterSize(18);
    m_header.setFillColor(Theme::NodeGlow);
    m_header.setString("EVENT LOG");
    m_header.setPosition(x + Pad, y + 6.f);

    m_scrollHint.setFont(font);
    m_scrollHint.setCharacterSize(12);
    m_scrollHint.setFillColor(Theme::StarOn);
    m_scrollHint.setString("scrolled - wheel down for latest");
    m_scrollHint.setPosition(x + Pad + 110.f, y + 11.f);

    m_measure.setFont(font);
    m_measure.setCharacterSize(TextSize);

    for (int i = 0; i < kMaxRows; ++i)
    {
        m_row[i].setFont(font);
        m_row[i].setCharacterSize(TextSize);
        m_row[i].setFillColor(Theme::TextColor);
        m_row[i].setPosition(x + Pad, y + HeaderH + (float)i * LineH);
    }

    m_scrollBar.setSize(sf::Vector2f(4.f, 20.f));
    m_scrollBar.setFillColor(Theme::NodeLine);
    m_dirty = true;
}

int EventLogPanel::rows() const
{
    int r = (int)((m_h - HeaderH - Pad) / LineH);
    if (r > kMaxRows)
    {
        r = kMaxRows;
    }
    if (r < 1)
    {
        r = 1;
    }
    return r;
}

void EventLogPanel::clear()
{
    m_start = 0;
    m_count = 0;
    m_scroll = 0;
    m_dirty = true;
}

void EventLogPanel::pushRaw(const std::string& line)
{
    if (m_count == kMaxLines)
    {
        m_lines[m_start] = line;
        m_start = (m_start + 1) % kMaxLines;
    }
    else
    {
        m_lines[(m_start + m_count) % kMaxLines] = line;
        m_count++;
    }
}

void EventLogPanel::pushWrapped(const std::string& line)
{
    float maxW = m_w - 2.f * Pad - 8.f;
    std::string cur;
    std::string word;
    bool first = true;
    size_t i = 0;
    while (i <= line.size())
    {
        char c = (i < line.size()) ? line[i] : ' ';
        if (c != ' ')
        {
            word += c;
        }
        else if (!word.empty())
        {
            std::string cand = cur.empty() ? word : (cur + " " + word);
            m_measure.setString(cand);
            if (m_measure.getLocalBounds().width > maxW && !cur.empty())
            {
                pushRaw(cur);
                first = false;
                cur = "  " + word; // continuation lines are indented
            }
            else
            {
                cur = cand;
            }
            word.clear();
        }
        ++i;
    }
    (void)first;
    pushRaw(cur);
}

void EventLogPanel::addLine(const std::string& line)
{
    pushWrapped(line);
    if (m_scroll > 0)
    {
        // keep the view where the player is reading (clamped in rebuild)
        m_scroll++;
    }
    m_dirty = true;
}

void EventLogPanel::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseWheelScrolled)
    {
        float mx = (float)e.mouseWheelScroll.x;
        float my = (float)e.mouseWheelScroll.y;
        if (mx >= m_x && mx <= m_x + m_w && my >= m_y && my <= m_y + m_h)
        {
            m_scroll += (e.mouseWheelScroll.delta > 0.f) ? 3 : -3;
            m_dirty = true;
        }
    }
}

void EventLogPanel::rebuild()
{
    int r = rows();
    int maxScroll = m_count - r;
    if (maxScroll < 0)
    {
        maxScroll = 0;
    }
    if (m_scroll > maxScroll)
    {
        m_scroll = maxScroll;
    }
    if (m_scroll < 0)
    {
        m_scroll = 0;
    }

    int endIdx = m_count - m_scroll; // exclusive
    int startIdx = endIdx - r;
    for (int i = 0; i < kMaxRows; ++i)
    {
        if (i >= r)
        {
            m_row[i].setString("");
            continue;
        }
        int li = startIdx + i;
        if (li < 0 || li >= m_count)
        {
            m_row[i].setString("");
        }
        else
        {
            m_row[i].setString(m_lines[(m_start + li) % kMaxLines]);
        }
    }

    // scroll bar thumb
    float trackH = m_h - HeaderH - Pad;
    if (m_count > r)
    {
        float frac = (float)r / (float)m_count;
        float thumbH = trackH * frac;
        if (thumbH < 16.f)
        {
            thumbH = 16.f;
        }
        float posFrac = (float)(startIdx) / (float)(m_count - r);
        m_scrollBar.setSize(sf::Vector2f(4.f, thumbH));
        m_scrollBar.setPosition(m_x + m_w - 10.f, m_y + HeaderH + posFrac * (trackH - thumbH));
    }
    else
    {
        m_scrollBar.setSize(sf::Vector2f(0.f, 0.f));
    }
    m_dirty = false;
}

void EventLogPanel::update(float dt)
{
    (void)dt;
    if (m_dirty)
    {
        rebuild();
    }
}

void EventLogPanel::draw(sf::RenderWindow& window) const
{
    window.draw(m_bg);
    window.draw(m_header);
    if (m_scroll > 0)
    {
        window.draw(m_scrollHint); // text label, not colour alone
    }
    int r = rows();
    for (int i = 0; i < r; ++i)
    {
        window.draw(m_row[i]);
    }
    window.draw(m_scrollBar);
}
