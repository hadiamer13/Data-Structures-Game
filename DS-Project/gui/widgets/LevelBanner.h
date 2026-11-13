// gui/widgets/LevelBanner.h
#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"   // reuses the ButtonCallback typedef (function pointer + void*)

// One tall banner card on the level select screen. Knows nothing about
// levels or saves: the owner sets number, stars, locked and selected state.
class LevelBanner {
public:
    // number e.g. "3" (the level inside the act). height = selected height.
    void init(const char* number, const sf::Font& font,
        const sf::Texture& banner, const sf::Texture& skull,
        sf::Vector2f center, float height, bool isBoss);

    // INTEGRATION (later): CampaignMap passes data from SaveManager here.
    void setState(int stars, bool unlocked);
    void setSelected(bool s) { m_selected = s; }

    void setCallback(ButtonCallback cb, void* context) {
        m_onClick = cb;
        m_context = context;
    }

    bool isHovered() const { return m_state != StateIdle; }

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

private:
    static const int StateIdle = 0;
    static const int StateHover = 1;
    static const int StatePressed = 2;

    void applyLayout(float s);   // s = current visual scale (about 0.8..1.05)
    void buildStars();

    sf::Vector2f m_center;
    float        m_height = 340.f;
    bool         m_boss = false;
    bool         m_unlocked = false;
    bool         m_selected = false;
    int          m_stars = 0;

    int   m_state = StateIdle;
    float m_hoverAmt = 0.f;     // 0..1 eased
    float m_selAmt = 0.f;       // 0..1 eased (drives scale and colour)
    float m_time = 0.f;         // glow pulse

    sf::Sprite m_glow;          // same banner texture, bigger, red, pulsing
    sf::Sprite m_banner;
    sf::Sprite m_skull;

    sf::Text m_head;            // "LEVEL"
    sf::Text m_num;             // big number
    sf::Text m_status;          // OPEN / LOCKED / CLEARED / BOSS

    sf::ConvexShape    m_star[3];
    sf::RectangleShape m_lockBody;      // padlock from shapes
    sf::CircleShape    m_lockShackle;

    ButtonCallback m_onClick = nullptr;
    void* m_context = nullptr;
};