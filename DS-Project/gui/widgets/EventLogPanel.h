// gui/widgets/EventLogPanel.h
// Scrolling text panel. Lines are wrapped when added and stored in a fixed
// ring (oldest dropped). Mouse wheel over the panel scrolls; 0 = follow latest.
// Reused by the Level screen and Act 4 (heap events).
#pragma once
#include <string>
#include <SFML/Graphics.hpp>

class EventLogPanel
{
  public:
    EventLogPanel();

    void init(const sf::Font& font, float x, float y, float w, float h);

    void clear();
    void addLine(const std::string& line); // long lines are wrapped

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

  private:
    void pushWrapped(const std::string& line);
    void pushRaw(const std::string& line);
    int rows() const;
    void rebuild();

    static const int kMaxLines = 300;
    static const int kMaxRows = 40;

    float m_x;
    float m_y;
    float m_w;
    float m_h;

    std::string m_lines[kMaxLines];
    int m_start; // ring index of the oldest line
    int m_count;
    int m_scroll; // lines scrolled up from the newest (0 = following)
    bool m_dirty;

    sf::RectangleShape m_bg;
    sf::RectangleShape m_scrollBar;
    sf::Text m_header;
    sf::Text m_scrollHint;
    sf::Text m_measure;
    sf::Text m_row[kMaxRows];
};
