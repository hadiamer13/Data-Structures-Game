#pragma once

#include "Screen.h"
#include "../AppContext.h"
#include "../Renderer.h"
#include "../widgets/Button.h"
#include "../widgets/NumberField.h"
#include "../../game/Leaderboard.h"
#include "../../game/SandboxSession.h"

//  endless screen
// Endless survival mode. Timer chalta hai, aur player ko sahi operation aur key dena hota hai.
// Score aur bache hue waqt ke hisaab se leaderboard mein score save hota hai.
class EndlessScreen : public Screen
{
  public:
    explicit EndlessScreen(AppContext& ctx);
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

  private:
    struct ActionContext
    {
        EndlessScreen* owner;
        int operation;
    };

    static void onAction(void* context);
    static void onHome(void* context);
    static void onRestart(void* context);
    void applyAction(int operation);
    void startRound();
    void finishGame();
    bool containsKey(int key) const;
    void removeKey(int key);

    AppContext& m_ctx;
    sf::Sprite m_background;
    sf::RectangleShape m_dim;
    sf::RectangleShape m_panel;
    sf::Text m_title;
    sf::Text m_prompt;
    sf::Text m_status;
    sf::Text m_scoreText;
    sf::Text m_timerText;
    sf::Text m_hint;
    Renderer m_renderer;
    SandboxSession m_session;
    IntArray m_keys;
    NumberField m_keyField;
    Button m_actions[3];
    ActionContext m_actionContexts[3];
    Button m_home;
    Button m_restart;
    float m_seconds;
    int m_score;
    int m_expectedOperation;
    int m_expectedKey;
    unsigned int m_random;
    bool m_gameOver;
    bool m_scoreSaved;
    std::string m_feedback;
};