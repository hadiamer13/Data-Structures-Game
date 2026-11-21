#include "CampaignMap.h"
#include "../UIManager.h"
#include "../widgets/Ornament.h"
#include "../Theme.h"
#include <cstdio>
#include <cmath>
#include <string>

namespace
{
    // Chhote math helpers, kyunke <algorithm> istemal nahi karna.
    // Small math helpers, since <algorithm> is not allowed.
    float minF(float a, float b)
    {
        return a < b ? a : b;
    }

    float maxF(float a, float b)
    {
        return a > b ? a : b;
    }

    // Har act ka sirf naam.
    // Only the name of each act.
    const char* const ActNames[4] =
        {
            "The Blueprint Wing", "The Sorted Wing",
            "The Architect's Wing", "The Heap Fortress"};

    // Text ko apne bounding box ke center par origin deta hai.
    // Puts the text origin at the centre of its bounding box.
    void centerOrigin(sf::Text& t)
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    }

    // Info panel ka title, jaise "1.1  Guard Patrol".
    // Title for the info panel, for example "1.1  Guard Patrol".
    std::string levelTitle(int act, int slot, const std::string& name)
    {
        return std::to_string(act + 1) + "." + std::to_string(slot + 1) + "  " + name;
    }
}

//  text helper
// Text ka common style set karta hai: font, size, rang aur halka outline.
// Sets the common text style: font, size, colour and a light outline.
void CampaignMap::styleText(sf::Text& t, const sf::Font& font, unsigned size,
                            const sf::Color& col, float x, float y)
{
    t.setFont(font);                          
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setOutlineColor(sf::Color(0, 0, 0, 200));
    t.setOutlineThickness(2.f);
    t.setPosition(x, y);
}

// Left ya right arrow banata hai, jiska tip cx, cy par hota hai.
// Builds a left or right arrow whose tip sits at cx, cy.
void CampaignMap::buildArrow(sf::ConvexShape& s, bool left, float cx, float cy)
{
    s.setPointCount(3);
    float d = left ? -1.f : 1.f;
    s.setPoint(0, sf::Vector2f(d * 14.f, 0.f)); // tip
    s.setPoint(1, sf::Vector2f(-d * 10.f, -16.f));
    s.setPoint(2, sf::Vector2f(-d * 10.f, 16.f));
    s.setPosition(cx, cy);
    s.setOutlineThickness(2.f);
}

// Teen stars ka shape banata hai.
// Builds three star shapes.
void CampaignMap::buildStars(float x, float y)
{
    const float outer = 14.f;
    const float inner = outer * 0.45f;
    for (int s = 0; s < 3; ++s)
    {
        m_star[s].setPointCount(10);
        for (int p = 0; p < 10; ++p)
        {
            float r = (p % 2 == 0) ? outer : inner;
            float a = -Theme::Pi / 2.f + p * (Theme::Pi / 5.f);
            m_star[s].setPoint(p, sf::Vector2f(r * std::cos(a), r * std::sin(a)));
        }
        m_star[s].setPosition(x + s * 36.f, y);
        m_star[s].setOutlineThickness(1.5f);
    }
}

//  construction
CampaignMap::CampaignMap(AppContext& ctx)
    : m_ctx(ctx), m_campaign(ctx.campaign), m_font(&ctx.assets->font())
{
    const sf::Font& font = *m_font;
    const float W = static_cast<float>(Theme::WindowW);
    const float H = static_cast<float>(Theme::WindowH);

    m_arrowAmt[0] = m_arrowAmt[1] = 0.f;
    m_arrowHover[0] = m_arrowHover[1] = false;

    //  background
    const sf::Texture& bg = ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / static_cast<float>(bg.getSize().x),
                          H / static_cast<float>(bg.getSize().y));
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 110));

    //  corner ornaments
    // Aik hi sprite ko char kono par mirror karke lagaya jata hai.
    // One sprite is mirrored to fill all four corners.
    {
        const sf::Texture& ct = ctx.assets->corner();
        const float inset = 12.f;
        const float cs = 70.f;
        placeCorner(m_corners[0], ct, inset, inset, cs, false, false);
        placeCorner(m_corners[1], ct, W - inset, inset, cs, true, false);
        placeCorner(m_corners[2], ct, inset, H - inset, cs, false, true);
        placeCorner(m_corners[3], ct, W - inset, H - inset, cs, true, true);
        for (int c = 0; c < 4; ++c)
        {
            m_corners[c].setColor(Theme::NodeLine);
        }
    }

    //  top bar tabs
    const float tabY = 30.f;
    for (int a = 0; a < ActCount; ++a)
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "ACT %d", a + 1);
        float cx = W / 2.f + (a - 1.5f) * 190.f;
        styleText(m_tab[a], font, 26, Theme::TextColor, cx, tabY);
        m_tab[a].setString(buf);
        centerOrigin(m_tab[a]);
        m_tab[a].setPosition(cx, tabY);
        m_tabRect[a] = sf::FloatRect(cx - 80.f, tabY - 20.f, 160.f, 40.f);
    }

    // Act highlighter ek T-shaped sprite hai, jo fixed width par scale hota hai.
    // The act highlighter is a T-shaped sprite scaled to a fixed width.
    const sf::Texture& hl = ctx.assets->highlight();
    const sf::Vector2u hs = hl.getSize();
    const float hlWidth = 120.f;
    const float hlScale = hlWidth / static_cast<float>(hs.x);
    m_tabHighlight.setTexture(hl);
    m_tabHighlight.setOrigin(hs.x / 2.f, 0.f);
    m_tabHighlight.setScale(hlScale, hlScale);
    m_tabHighlight.setColor(Theme::NodeGlow);

    // Arrows pehle aur aakhri tab ke bahar lagte hain.
    // Arrows sit just outside the first and last tab.
    float lx = W / 2.f - 1.5f * 190.f - 130.f;
    float rx = W / 2.f + 1.5f * 190.f + 130.f;
    buildArrow(m_arrow[0], true, lx, tabY);
    buildArrow(m_arrow[1], false, rx, tabY);
    m_arrowRect[0] = sf::FloatRect(lx - 25.f, tabY - 25.f, 50.f, 50.f);
    m_arrowRect[1] = sf::FloatRect(rx - 25.f, tabY - 25.f, 50.f, 50.f);

    styleText(m_actName, font, 18, Theme::NodeGlow, W / 2.f, 54.f);

    // Act ke naam ke neeche divider.
    // Divider under the act name.
    placeDivider(m_divider, ctx.assets->divider(), W / 2.f, 88.f, 400.f, 18.f);
    m_divider.setColor(Theme::NodeGlow);

    // Positions, numbers, boss flags aur states initBanners() aur loadBannerStates() se aate hain,
    // jo setAct() call karta hai. Yahan sirf callbacks set hote hain.
    // Positions, numbers, boss flags and states come from initBanners() and loadBannerStates(),
    // which setAct() calls. Only the callbacks are set here.
    for (int i = 0; i < PerAct; ++i)
    {
        m_bannerCtx[i].map = this;
        m_bannerCtx[i].slot = i;
        m_banner[i].setCallback(&CampaignMap::onBannerClick, &m_bannerCtx[i]);
    }

    
    // Info panel banners ke neeche hai.
    // The info panel sits under the banners.
    m_panelX = 100.f;  m_panelY = 448.f;
    m_panelW = W - 200.f;  m_panelH = 252.f;
    m_panel.setSize(sf::Vector2f(m_panelW, m_panelH));
    m_panel.setPosition(m_panelX, m_panelY);
    m_panel.setFillColor(Theme::TooltipBg);
    m_panel.setOutlineColor(Theme::NodeLine);
    m_panel.setOutlineThickness(3.f);

    const float tx = m_panelX + 30.f;
    styleText(m_title, font, 34, Theme::TitleColor, tx, m_panelY + 14.f);
    styleText(m_status, font, 18, Theme::NodeGlow, tx, m_panelY + 60.f);
    styleText(m_concept, font, 20, Theme::TextColor, tx, m_panelY + 92.f);
    styleText(m_objective, font, 20, Theme::TextColor, tx, m_panelY + 126.f);

    const float rx2 = m_panelX + m_panelW - 280.f;
    styleText(m_bestLabel, font, 18, Theme::NodeGlow, rx2, m_panelY + 60.f);
    buildStars(rx2 + 90.f, m_panelY + 71.f);
    styleText(m_lockedMsg, font, 18, Theme::NodeLockedLine, rx2, m_panelY + 150.f);
    m_lockedMsg.setString("LOCKED\nFinish the previous level\nto unlock this one.");

    //  buttons
    m_start.init("Start", font,
                 sf::Vector2f(m_panelX + m_panelW - 150.f, m_panelY + m_panelH - 55.f),
                 sf::Vector2f(240.f, Theme::ButtonH));
    m_start.setCallback(&CampaignMap::onStartClick, this);
    m_start.setFrame(&ctx.assets->buttonFrame());

    m_back.init("Back", font, sf::Vector2f(110.f, tabY),
                sf::Vector2f(150.f, 42.f));
    m_back.setCallback(&CampaignMap::onBackClick, this);
    m_back.setFrame(&ctx.assets->buttonFrame());

    // Debug button info panel ke bottom-left mein hai, text aur Start se door.
    // The debug button sits at the bottom left of the info panel, away from the text and Start.
    m_debug.init("DEBUG: UNLOCK ALL (OFF)", font,
                 sf::Vector2f(m_panelX + 250.f, m_panelY + m_panelH - 55.f),
                 sf::Vector2f(260.f, Theme::ButtonH));
    m_debug.setCallback(&CampaignMap::onDebugClick, this);
    m_debug.setFrame(&ctx.assets->buttonFrame());
    refreshDebugLabel();

    //  restore position
    // Briefing se wapas aane par wahi level select rahe jo pehle tha.
    // When coming back from the briefing, keep the same level selected.
    int act = 0;
    int sel = 0;
    if (ctx.selectedLevel >= 0 && ctx.selectedLevel < ActCount * PerAct)
    {
        act = ctx.selectedLevel / PerAct;
        sel = ctx.selectedLevel % PerAct;
    }
    m_act = act;
    m_sel = sel;
    setAct(act);
    setSelected(sel);
}

//  banners setup
// Current act ke liye banner number, boss flag aur layout set karta hai.
// Sets the banner numbers, boss flags and layout for the current act.
void CampaignMap::initBanners()
{
    const float W = static_cast<float>(Theme::WindowW);
    for (int i = 0; i < PerAct; ++i)
    {
        char num[8];
        std::snprintf(num, sizeof(num), "%d", i + 1);
        const bool boss = m_campaign->isBoss(m_act * PerAct + i);
        m_banner[i].init(num, *m_font, m_ctx.assets->banner(), m_ctx.assets->skull(),
                         sf::Vector2f(W / 2.f + (i - 2) * 215.f, 276.f),
                         Theme::BannerH, boss);
    }
}

void CampaignMap::loadBannerStates()
{
    for (int s = 0; s < PerAct; ++s)
    {
        const int i = m_act * PerAct + s;
        m_banner[s].setState(m_campaign->stars(i), m_campaign->isUnlocked(i));
    }
}

void CampaignMap::refreshFromData()
{
    loadBannerStates();
    refreshInfo();
}

// Act badalta hai, tabs ka rang badalta hai, aur pehla unplayed level select hota hai.
// Changes the act, updates the tab colours, and selects the first unplayed level.
void CampaignMap::setAct(int act)
{
    if (act < 0)
    {
        act = 0;
    }
    if (act >= ActCount)
    {
        act = ActCount - 1;
    }
    m_act = act;

    const float W = static_cast<float>(Theme::WindowW);
    initBanners();
    loadBannerStates();

    // Tab ke rang
    // Tab colours
    for (int a = 0; a < ActCount; ++a)
    {
        m_tab[a].setFillColor(a == act ? Theme::NodeGlow : Theme::TextColor);
    }
    m_tabHighlight.setPosition(m_tab[act].getPosition().x,
                               m_tab[act].getPosition().y + 26.f);

    m_actName.setString(ActNames[act]);
    centerOrigin(m_actName);
    m_actName.setPosition(W / 2.f, 54.f);

    // Pehla level jo abhi khela nahi gaya, woh select hota hai, warna pehla.
    // Select the first unlocked level with no stars yet, otherwise the first level.
    int pick = 0;
    for (int s = 0; s < PerAct; ++s)
    {
        const int i = act * PerAct + s;
        if (m_campaign->isUnlocked(i) && m_campaign->stars(i) == 0)
        {
            pick = s;
            break;
        }
    }
    setSelected(pick);
}

void CampaignMap::setSelected(int slot)
{
    if (slot < 0)
    {
        slot = 0;
    }
    if (slot >= PerAct)
    {
        slot = PerAct - 1;
    }
    m_sel = slot;
    for (int s = 0; s < PerAct; ++s)
    {
        m_banner[s].setSelected(s == slot);
    }
    refreshInfo();
}

// Start tabhi enable hota hai jab data load ho aur level unlock ho.
// Start is enabled only when the data is loaded and the level is unlocked.
bool CampaignMap::startEnabled() const
{
    return m_campaign->loaded() && m_campaign->isUnlocked(levelIndex());
}

void CampaignMap::refreshDebugLabel()
{
    m_debug.setLabel(m_campaign->unlockAll() ? "DEBUG: UNLOCK ALL (ON)" : "DEBUG: UNLOCK ALL (OFF)");
}

// Info panel ke strings sirf tab badalte hain jab state badle, har frame mein nahi.
// Info panel strings change only when the state changes, not every frame.
void CampaignMap::refreshInfo()
{
    if (!m_campaign->loaded())
    {
        m_title.setString("CAMPAIGN DATA MISSING");
        m_status.setString("ERROR");
        m_concept.setString(m_campaign->error());
        m_objective.setString("Check that levels/act1.json to act4.json sit next to the game.");
        m_bestLabel.setString("");
        for (int s = 0; s < 3; ++s)
        {
            m_star[s].setFillColor(sf::Color(0, 0, 0, 0));
            m_star[s].setOutlineColor(Theme::StarOff);
        }
        return;
    }

    const int i = levelIndex();
    const LevelDef& def = m_campaign->level(i);
    const bool unlocked = m_campaign->isUnlocked(i);
    const int stars = unlocked ? m_campaign->stars(i) : 0;
    const bool boss = m_campaign->isBoss(i);

    m_title.setString(levelTitle(m_act, m_sel, def.name));
    m_concept.setString(def.conceptText);
    m_objective.setString(def.objective);

    // Status ek word hai, sirf rang nahi.
    // The status is a word, not only a colour.
    const char* word = "OPEN";
    if (!unlocked)
    {
        word = "LOCKED";
    }
    else if (stars > 0)
    {
        word = boss ? "CLEARED  -  BOSS LEVEL" : "CLEARED";
    }
    else if (boss)
    {
        word = "OPEN  -  BOSS LEVEL";
    }
    m_status.setString(word);

    m_bestLabel.setString("BEST");
    for (int s = 0; s < 3; ++s)
    {
        const bool on = s < stars;
        m_star[s].setFillColor(on ? Theme::StarOn : sf::Color(0, 0, 0, 0));
        m_star[s].setOutlineColor(on ? Theme::StarOn : Theme::StarOff);
    }
}

//  callbacks
void CampaignMap::onBannerClick(void* context)
{
    BannerCtx* c = static_cast<BannerCtx*>(context);
    c->map->onBannerClicked(c->slot);
}

void CampaignMap::onStartClick(void* context)
{
    static_cast<CampaignMap*>(context)->onStart();
}

void CampaignMap::onBackClick(void* context)
{
    static_cast<CampaignMap*>(context)->onBack();
}

void CampaignMap::onDebugClick(void* context)
{
    static_cast<CampaignMap*>(context)->onDebugUnlock();
}

// Pehla click select karta hai, selected banner par dobara click Briefing kholta hai.
// The first click selects. Clicking the selected banner again opens the briefing.
void CampaignMap::onBannerClicked(int slot)
{
    if (slot == m_sel)
    {
        onStart();
    }
    else
    {
        setSelected(slot);
    }
}

void CampaignMap::onStart()
{
    if (!startEnabled())
    {
        return;      // locked ya data missing ho to kuch nahi hota
    }
    m_ctx.selectedLevel = levelIndex();
    m_ctx.ui->requestScreen(ScreenId::Briefing);
}

void CampaignMap::onBack()
{
    m_ctx.ui->requestScreen(ScreenId::MainMenu);
}

void CampaignMap::onDebugUnlock()
{
    if (!m_campaign->loaded())
    {
        return;
    }
    m_campaign->toggleUnlockAll();
    loadBannerStates();
    refreshDebugLabel();
    refreshInfo();
}

//  input
// Esc UIManager handle karta hai. Yahan keys, mouse aur hover ko sambhalte hain.
// Esc is handled by UIManager. Keys, mouse and hover are handled here.
void CampaignMap::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::KeyPressed)
    {
        sf::Keyboard::Key k = e.key.code;
        if (k == sf::Keyboard::Left || k == sf::Keyboard::Q)
        {
            setAct(m_act - 1);
        }
        if (k == sf::Keyboard::Right || k == sf::Keyboard::E)
        {
            setAct(m_act + 1);
        }
        if (k == sf::Keyboard::Return)
        {
            onStart();
        }
    }

    if (e.type == sf::Event::MouseMoved)
    {
        float mx = static_cast<float>(e.mouseMove.x);
        float my = static_cast<float>(e.mouseMove.y);
        m_tabHover = -1;
        for (int a = 0; a < ActCount; ++a)
        {
            if (m_tabRect[a].contains(mx, my))
            {
                m_tabHover = a;
            }
        }
        for (int s = 0; s < 2; ++s)
        {
            m_arrowHover[s] = m_arrowRect[s].contains(mx, my);
        }
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        float mx = static_cast<float>(e.mouseButton.x);
        float my = static_cast<float>(e.mouseButton.y);
        if (m_arrowRect[0].contains(mx, my))
        {
            setAct(m_act - 1);
        }
        if (m_arrowRect[1].contains(mx, my))
        {
            setAct(m_act + 1);
        }
        for (int a = 0; a < ActCount; ++a)
        {
            if (m_tabRect[a].contains(mx, my))
            {
                setAct(a);
            }
        }
    }

    for (int s = 0; s < PerAct; ++s)
    {
        m_banner[s].handleEvent(e);
    }
    m_back.handleEvent(e);
    m_debug.handleEvent(e);
    if (startEnabled())
    {
        m_start.handleEvent(e);
    }
}

//  update
// Hover ke scale aur arrows ki animation har frame par chalti hai.
// Hover scale and arrow animation run every frame.
void CampaignMap::update(float dt)
{
    for (int s = 0; s < PerAct; ++s)
    {
        m_banner[s].update(dt);
    }
    m_back.update(dt);
    m_debug.update(dt);
    m_start.update(dt);

    // Tab hover par roshan hota hai, selected act hamesha highlight rehta hai.
    // A hovered tab brightens, and the selected act stays highlighted.
    for (int a = 0; a < ActCount; ++a)
    {
        if (a == m_act)
        {
            m_tab[a].setFillColor(Theme::NodeGlow);
        }
        else if (a == m_tabHover)
        {
            m_tab[a].setFillColor(Theme::BannerHover);
        }
        else
        {
            m_tab[a].setFillColor(Theme::TextColor);
        }
    }

    // Arrows ka hover smooth hota hai. Aakhri act par arrow chhota aur bina outline ke dikhta hai,
    // taake rang ke alawa bhi farq nazar aaye.
    // Arrows ease their hover scale. At the ends the arrow is smaller and has no outline,
    // so the difference shows without relying on colour.
    for (int s = 0; s < 2; ++s)
    {
        float t = m_arrowHover[s] ? 1.f : 0.f;
        float st = 8.f * dt;
        if (m_arrowAmt[s] < t)
        {
            m_arrowAmt[s] = minF(t, m_arrowAmt[s] + st);
        }
        else
        {
            m_arrowAmt[s] = maxF(t, m_arrowAmt[s] - st);
        }

        bool atEnd = (s == 0 && m_act == 0) || (s == 1 && m_act == ActCount - 1);
        float sc = (atEnd ? 0.7f : 1.f) + 0.25f * m_arrowAmt[s];
        m_arrow[s].setScale(sc, sc);
        sf::Color fill = atEnd ? Theme::StarOff : Theme::NodeLine;
        if (!atEnd && m_arrowAmt[s] > 0.5f)
        {
            fill = Theme::NodeGlow;
        }
        m_arrow[s].setFillColor(fill);
        m_arrow[s].setOutlineColor(atEnd ? sf::Color(0, 0, 0, 0) : sf::Color(0, 0, 0, 200));
    }
}

//  draw
// Pehle backdrop, phir top bar, banners, aur aakhir mein info panel.
// Backdrop first, then the top bar, the banners, and the info panel last.
void CampaignMap::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    for (int c = 0; c < 4; ++c)
    {
        window.draw(m_corners[c]);
    }

    m_back.draw(window);
    for (int a = 0; a < ActCount; ++a)
    {
        window.draw(m_tab[a]);
    }
    window.draw(m_tabHighlight);
    window.draw(m_arrow[0]);
    window.draw(m_arrow[1]);
    window.draw(m_actName);
    window.draw(m_divider);

    // Selected banner sab se upar draw hota hai, taake uska glow aur scale sab par nazar aaye.
    // The selected banner is drawn last, so its glow and scale sit on top.
    for (int s = 0; s < PerAct; ++s)
    {
        if (s != m_sel)
        {
            m_banner[s].draw(window);
        }
    }
    m_banner[m_sel].draw(window);

    window.draw(m_panel);
    window.draw(m_title);
    window.draw(m_status);
    window.draw(m_concept);
    window.draw(m_objective);
    window.draw(m_bestLabel);
    for (int s = 0; s < 3; ++s)
    {
        window.draw(m_star[s]);
    }

    m_debug.draw(window);
    if (startEnabled())
    {
        m_start.draw(window);
    }
    else
    {
        window.draw(m_lockedMsg);
    }
}