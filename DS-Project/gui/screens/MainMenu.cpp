// gui/screens/MainMenu.cpp
#include "MainMenu.h"
#include "../UIManager.h"
#include "../Theme.h"

namespace {
    const int QuitTarget = -2;   // not a screen: tells UIManager to quit

    // Label + destination for each button, top to bottom.
    const char* const Labels[6] = {
        "Campaign", "Endless", "Sandbox", "Leaderboard", "Settings", "Quit"
    };
    const int Targets[6] = {
        ScreenId::Campaign, ScreenId::Endless, ScreenId::Sandbox,
        ScreenId::Leaderboard, ScreenId::Settings, QuitTarget
    };
}  // namespace

MainMenu::MainMenu(AppContext& ctx) : m_ctx(ctx) {
    const sf::Font& font = ctx.assets->font();

    // ---- Background -------------------------------------------------
    // TODO(YOU): your image path is set in Theme.h (MenuBgPath) and
    // loaded in Assets.cpp. Nothing to change here.
    const sf::Texture& bg = ctx.assets->menuBackground();
    m_background.setTexture(bg);
    // Stretch to the window. If your image has the same aspect ratio as
    // the window this is exact; otherwise adjust Theme::WindowW/H.
    m_background.setScale(
        static_cast<float>(Theme::WindowW) / static_cast<float>(bg.getSize().x),
        static_cast<float>(Theme::WindowH) / static_cast<float>(bg.getSize().y));

    // ---- Title ------------------------------------------------------
    m_title.setFont(font);
    m_title.setString("VAULT HEIST");
    m_title.setCharacterSize(Theme::TitleSize);
    m_title.setFillColor(Theme::TitleColor);
    m_title.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_title.setOutlineThickness(3.f);
    sf::FloatRect tb = m_title.getLocalBounds();
    m_title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    m_title.setPosition(Theme::WindowW / 2.f, 90.f);

    m_subtitle.setFont(font);
    m_subtitle.setString("The Living Vault");
    m_subtitle.setCharacterSize(28);
    m_subtitle.setFillColor(Theme::TextColor);
    m_subtitle.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_subtitle.setOutlineThickness(2.f);
    sf::FloatRect sb = m_subtitle.getLocalBounds();
    m_subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    m_subtitle.setPosition(Theme::WindowW / 2.f, 150.f);

    // ---- Buttons ----------------------------------------------------
    // TODO(YOU): tweak firstY if your background art needs the buttons
    // higher/lower. 6 buttons x 74 px must fit inside WindowH.
    const float cx = Theme::WindowW / 2.f;
    const float firstY = 240.f;
    const float stepY = Theme::ButtonH + Theme::ButtonGap;

    for (int i = 0; i < ButtonCount; ++i) {
        m_btnCtx[i].menu = this;
        m_btnCtx[i].target = Targets[i];

        m_buttons[i].init(Labels[i], font,
            sf::Vector2f(cx, firstY + stepY * i),
            sf::Vector2f(Theme::ButtonW, Theme::ButtonH));
        m_buttons[i].setCallback(&MainMenu::onButtonClick, &m_btnCtx[i]);
    }
}

void MainMenu::onButtonClick(void* context) {
    ButtonCtx* c = static_cast<ButtonCtx*>(context);
    c->menu->onPressed(c->target);
}

void MainMenu::onPressed(int target) {
    // INTEGRATION POINT: this is the only place the menu decides where to go.
    // Later, e.g. Campaign can first load progress via SaveManager before
    // switching. For now it just asks UIManager (deferred switch).
    if (target == QuitTarget) m_ctx.ui->requestQuit();
    else                      m_ctx.ui->requestScreen(target);
}

void MainMenu::handleEvent(const sf::Event& e) {
    for (int i = 0; i < ButtonCount; ++i) m_buttons[i].handleEvent(e);
}

void MainMenu::update(float dt) {
    for (int i = 0; i < ButtonCount; ++i) m_buttons[i].update(dt);
}

void MainMenu::draw(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_title);
    window.draw(m_subtitle);
    for (int i = 0; i < ButtonCount; ++i) m_buttons[i].draw(window);
}