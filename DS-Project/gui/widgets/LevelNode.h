// gui/widgets/LevelNode.h
#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"   // reuses the ButtonCallback typedef (function pointer + void*)

// One level node on the campaign map. Knows nothing about levels or saves:
// the owner tells it its label, stars and locked state, and gets a callback.
class LevelNode {
public:
    // label e.g. "2.3". center = node centre in window pixels.
    void init(const char* label, const sf::Font& font,
        sf::Vector2f center, float radius, bool isBoss);

    // INTEGRATION (later): CampaignMap calls this with data from SaveManager.
    // stars: 0..3 (0 = not completed). unlocked: false shows the padlock.
    void setState(int stars, bool unlocked);

    void setCallback(ButtonCallback cb, void* context) {
        m_onClick = cb;
        m_context = context;
    }

    bool isHovered() const { return m_state != StateIdle; }
    sf::Vector2f center() const { return m_center; }
    float radius() const { return m_radius; }

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    // States as plain ints (no enums).
    static const int StateIdle = 0;
    static const int StateHover = 1;
    static const int StatePressed = 2;

    static const int DashCount = 20;   // dashed outline when locked

    bool contains(int x, int y) const;
    void buildStars();
    void buildDashes();
    void layoutLabel();

    sf::Vector2f m_center;
    float        m_radius = 30.f;
    bool         m_boss = false;
    bool         m_unlocked = false;
    int          m_stars = 0;

    int   m_state = StateIdle;
    float m_hoverAmt = 0.f;   // 0..1 eased
    float m_time = 0.f;       // drives the glow pulse

    sf::CircleShape    m_glow;       // pulsing highlight ring
    sf::CircleShape    m_body;
    sf::CircleShape    m_bossRing;   // second outline for boss levels
    sf::RectangleShape m_dash[DashCount];
    sf::Text           m_label;
    sf::Text           m_bossText;

    // Stars (3 slots below the node)
    sf::ConvexShape    m_star[3];

    // Padlock (drawn from shapes, no image needed)
    sf::RectangleShape m_lockBody;
    sf::CircleShape    m_lockShackle;

    ButtonCallback m_onClick = nullptr;
    void* m_context = nullptr;
};