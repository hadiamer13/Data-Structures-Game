// gui/screens/LevelScreen.cpp
#include "LevelScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../EventText.h"
#include "../../game/LevelLoader.h"

namespace
{
    const float HudH = 84.f;
    const float ToolbarH = 48.f;
    const float LogW = 320.f;
    const float BarH = 64.f;
    const float ClickSlop = 5.f;

    bool structural(int type)
    {
        return type == EventType::INSERT || type == EventType::UNLINK;
    }

    void styleText(sf::Text& t, const sf::Font& font, unsigned size, const sf::Color& col, float x, float y)
    {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setPosition(x, y);
    }
}

// ------------------------------------------------------------ construction

LevelScreen::LevelScreen(AppContext& ctx)
    : m_ctx(ctx), m_index(0), m_ok(false), m_shownAlarm(0), m_endTimer(0.f), m_selected(kNull),
    m_renderer(ctx.assets->font()), m_anim(&m_renderer),
    m_actCount(0), m_pressX(0), m_pressY(0), m_pressing(false)
{
    m_index = ctx.selectedLevel;
    if (m_index < 0 || m_index > 19)
    {
        m_index = 0;
    }
    for (int i = 0; i < 3; ++i)
    {
        m_actCtx[i].owner = this;
        m_actCtx[i].which = i;
    }

    buildLayout();
    m_anim.setCallbacks(&LevelScreen::onAnimEvent, &LevelScreen::onAnimDone, this);
    m_ok = loadLevel();

    if (m_ok)
    {
        m_renderer.snapTo(m_session.snapshot());
        m_renderer.fit(false);
        buildActionButtons();
        refreshHud(m_session.hud());
        m_log.addLine("Level ready. Type a key, then use the buttons below.");
        m_log.addLine("Wheel = zoom, drag background = pan, hover = info.");
    }
    else
    {
        m_hud.setTitle(m_def.name.empty() ? std::string("Level") : m_def.name);
        m_hud.setObjective(m_error);
        m_hud.setStatus("UNAVAILABLE");
        m_notice.setString(m_error + "\n\nPlayable now: 2.1 Search, 2.2 Insert, 2.3 Delete, 2.5 Boss.\nPress Esc to go back.");
    }
}

void LevelScreen::buildLayout()
{
    const sf::Font& font = m_ctx.assets->font();
    const float W = (float)Theme::WindowW;
    const float H = (float)Theme::WindowH;

    const sf::Texture& bg = m_ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / (float)bg.getSize().x, H / (float)bg.getSize().y);
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 190));

    float cx = 0.f;
    float cy = HudH;
    float cw = W - LogW;
    float ch = H - HudH - ToolbarH - BarH;

    m_canvasBox.setPosition(cx, cy);
    m_canvasBox.setSize(sf::Vector2f(cw, ch));
    m_canvasBox.setFillColor(sf::Color(12, 6, 9, 170));
    m_canvasBox.setOutlineColor(Theme::BtnOutline);
    m_canvasBox.setOutlineThickness(-2.f);
    m_renderer.setCanvasRect(cx, cy, cw, ch);

    styleText(m_hint, font, 12, sf::Color(170, 150, 150), cx + 10.f, cy + ch - 20.f);
    m_hint.setString("Wheel: zoom   Drag: pan   Click a room: fills KEY   Space: pause   Z: undo   H: hint");
    styleText(m_msg, font, 15, Theme::StarOn, cx + 10.f, cy + 8.f);
    styleText(m_notice, font, 24, Theme::TextColor, cx + 40.f, cy + 80.f);

    float barY = H - ToolbarH - BarH;
    m_bar.setPosition(0.f, barY);
    m_bar.setSize(sf::Vector2f(cw, BarH));
    m_bar.setFillColor(Theme::TooltipBg);
    m_bar.setOutlineColor(Theme::BtnOutline);
    m_bar.setOutlineThickness(-2.f);

    m_field.init("KEY", font, 16.f, barY + 28.f, 130.f, 28.f);
    m_field.setCallback(&LevelScreen::onFieldClick, this);
    styleText(m_targetText, font, 18, Theme::StarOn, 165.f, barY + 22.f);

    m_hud.init(font, 0.f, 0.f, W, HudH);
    m_log.init(font, W - LogW, HudH, LogW, H - HudH - ToolbarH);
    m_toolbar.init(font, 0.f, H - ToolbarH, W, ToolbarH);
    m_toolbar.setCallback(&LevelScreen::onToolbarClick, this);
    m_tooltip.init(font);
}

// Level definition comes from levels/actN.json (LevelLoader), index = act * 5 + level.
bool LevelScreen::loadLevel()
{
    const int act = m_index / 5 + 1;
    const int slot = m_index % 5;
    const std::string path = std::string("levels/act") + static_cast<char>('0' + act) + ".json";

    LevelDefArray defs;
    std::string error;
    if (!LevelLoader::loadFile(path, defs, &error))
    {
        m_error = "Could not load " + path + ": " + error;
        return false;
    }
    m_def = defs[slot];

    if (!GameSession::supportsMission(m_def.mission) && !GameSession::isBinaryMission(m_def.mission))
    {
        m_error = "Level " + std::to_string(act) + "." + std::to_string(slot + 1) + " is not implemented yet.";
        return false;
    }
    if (GameSession::isHeapMission(m_def.mission))
    {
        m_error = "The Heap Fortress needs the HeapLevel screen (next step).";
        return false;
    }
    if (!m_session.startLevel(m_def))
    {
        m_error = "The level data for \"" + m_def.name + "\" could not be started.";
        return false;
    }

    m_hud.setTitle(std::to_string(act) + "." + std::to_string(slot + 1) + " " + m_def.name);
    std::string objective = m_def.objective;
    if (m_session.targetKey() != kNull)
    {
        objective += "  Target key: " + std::to_string(m_session.targetKey()) + ".";
        m_targetText.setString("Target: " + std::to_string(m_session.targetKey()));
    }
    m_hud.setObjective(objective);
    return true;
}

void LevelScreen::buildActionButtons()
{
    const sf::Font& font = m_ctx.assets->font();
    float barY = (float)Theme::WindowH - ToolbarH - BarH;
    const int mission = m_def.mission;

    const char* labels[3] = { "", "", "" };
    if (mission == MissionKind::BstDelete)
    {
        labels[0] = "Delete: leaf";
        labels[1] = "Delete: one child";
        labels[2] = "Delete: two children";
        m_actCount = 3;
    }
    else if (mission == MissionKind::BstInsert)
    {
        labels[0] = "Insert";
        m_actCount = 1;
    }
    else
    {
        labels[0] = "Search";
        m_actCount = 1;
    }

    for (int i = 0; i < m_actCount; ++i)
    {
        float cx = 410.f + (float)i * 215.f;
        m_act[i].init(labels[i], font, sf::Vector2f(cx, barY + 34.f), sf::Vector2f(205.f, 38.f));
        m_act[i].setCallback(&LevelScreen::onActionClick, &m_actCtx[i]);
    }
}

// ---------------------------------------------------------------- callbacks

void LevelScreen::onToolbarClick(void* context, int action)
{
    static_cast<LevelScreen*>(context)->onToolbar(action);
}

void LevelScreen::onActionClick(void* context)
{
    ActCtx* c = static_cast<ActCtx*>(context);
    c->owner->onAction(c->which);
}

void LevelScreen::onFieldClick(void* context)
{
    static_cast<LevelScreen*>(context)->onFieldEnter();
}

void LevelScreen::onAnimEvent(void* context, const Event& e, int index, int total)
{
    (void)index;
    (void)total;
    static_cast<LevelScreen*>(context)->animEvent(e);
}

void LevelScreen::onAnimDone(void* context)
{
    static_cast<LevelScreen*>(context)->animDone();
}

void LevelScreen::onAction(int which)
{
    doAction(which);
}

void LevelScreen::onFieldEnter()
{
    if (m_def.mission != MissionKind::BstDelete)
    {
        doAction(0);       // Enter = the single action; delete needs an explicit case
    }
    else
    {
        setMessage("Pick the delete case with a button.");
    }
}

void LevelScreen::onToolbar(int action)
{
    if (!m_ok)
    {
        return;
    }
    if (action == ToolbarAction::Undo)
    {
        doUndo();
    }
    else if (action == ToolbarAction::Replay)
    {
        if (m_anim.totalCount() > 0)
        {
            m_log.addLine("--- replay ---");
            m_shownAlarm = m_lastHud.alarm - m_anim.totalCount();   // display only, corrected at the end
            if (m_shownAlarm < 0)
            {
                m_shownAlarm = 0;
            }
            m_anim.startReplay();
        }
        else
        {
            setMessage("Nothing to replay yet.");
        }
    }
    else if (action == ToolbarAction::Reset)
    {
        doReset();
    }
    else if (action == ToolbarAction::Hint)
    {
        doHint();
    }
}

// ------------------------------------------------------------------- HUD

void LevelScreen::setMessage(const std::string& msg)
{
    m_msg.setString(msg);
    if (!msg.empty())
    {
        m_log.addLine("> " + msg);
    }
}

// Numbers come from the session's HudState, never from GUI maths.
void LevelScreen::refreshHud(const HudState& h)
{
    m_lastHud = h;
    m_shownAlarm = h.alarm;
    m_hud.setAlarm(h.alarm, h.alarmCap);
    m_hud.setScore(h.score);
    m_hud.setSteps(h.alarm, h.alarmCap);
    refreshStatus();
}

void LevelScreen::refreshStatus()
{
    if (m_anim.busy() && m_anim.paused())
    {
        m_hud.setStatus("PAUSED");
    }
    else if (m_anim.busy())
    {
        m_hud.setStatus("PLAYING");
    }
    else if (m_lastHud.status == HudStatus::Won)
    {
        m_hud.setStatus("CLEARED");
    }
    else if (m_lastHud.status == HudStatus::Lost)
    {
        m_hud.setStatus("ALARM FULL");
    }
    else
    {
        m_hud.setStatus("READY");
    }
}

bool LevelScreen::typing() const
{
    return m_field.focused();
}

// ------------------------------------------------------------------ actions

void LevelScreen::doAction(int which)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;      // input is locked while the animation plays
    }
    if (!m_field.hasValue())
    {
        setMessage("Type a key in the KEY box first.");
        return;
    }

    PlayerAction act;
    act.a = m_field.value();
    if (m_def.mission == MissionKind::BstDelete)
    {
        act.kind = ActionKind::Delete;
        act.c = (which == 0) ? DeleteCase::Leaf : ((which == 1) ? DeleteCase::OneChild : DeleteCase::TwoChildren);
    }
    else if (m_def.mission == MissionKind::BstInsert)
    {
        act.kind = ActionKind::Insert;
    }
    else
    {
        act.kind = ActionKind::Search;
    }

    m_renderer.setTarget(kNull);
    ViewSnapshot before = m_session.snapshot();
    ActionResult r = m_session.apply(act);
    setMessage(r.message);
    play(before, r);
}

// One batch. The session returns the final state; the last structural event (INSERT / UNLINK)
// carries the "after" snapshot so the tree tweens exactly when that event plays.
void LevelScreen::play(const ViewSnapshot& before, const ActionResult& r)
{
    ViewSnapshot after = m_session.snapshot();
    m_lastHud = r.hud;
    m_shownAlarm = 0;
    m_shownAlarm = r.hud.alarm;
    for (int i = 0; i < r.events.size(); ++i)
    {
        m_shownAlarm -= r.events[i].alarmCost;
    }
    if (m_shownAlarm < 0)
    {
        m_shownAlarm = 0;
    }

    int lastStruct = -1;
    for (int i = 0; i < r.events.size(); ++i)
    {
        if (structural(r.events[i].type))
        {
            lastStruct = i;
        }
    }

    m_renderer.snapTo(before);
    m_anim.beginBatch(before);
    for (int i = 0; i < r.events.size(); ++i)
    {
        m_anim.enqueue(r.events[i], (i == lastStruct) ? &after : nullptr);
    }
    m_log.addLine("--- play ---");
    m_anim.endBatch(after);
    refreshStatus();
}

void LevelScreen::animEvent(const Event& e)
{
    m_log.addLine(describeEventText(e));
    m_shownAlarm += e.alarmCost;
    m_hud.setAlarm(m_shownAlarm, m_lastHud.alarmCap);
    m_hud.setSteps(m_shownAlarm, m_lastHud.alarmCap);
    refreshStatus();
}

void LevelScreen::animDone()
{
    refreshHud(m_lastHud);     // authoritative numbers from the session
    if (m_lastHud.status != HudStatus::Playing)
    {
        m_log.addLine(m_lastHud.status == HudStatus::Won ? "LEVEL CLEARED" : "ALARM FULL");
        m_endTimer = 1.0f;     // Result screen only after the animation ends
    }
}

void LevelScreen::doUndo()
{
    if (m_endTimer > 0.f)
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Undo;
    ActionResult r = m_session.apply(act);
    m_anim.clear();
    m_renderer.snapTo(m_session.snapshot());     // hard sync, no stale positions
    m_renderer.setTarget(kNull);
    m_renderer.setSelected(kNull);
    m_selected = kNull;
    refreshHud(r.hud);
    setMessage(r.message);
}

void LevelScreen::doReset()
{
    if (m_endTimer > 0.f)
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Reset;
    ActionResult r = m_session.apply(act);
    m_anim.clear();
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setTarget(kNull);
    m_renderer.setSelected(kNull);
    m_renderer.fit(true);
    m_selected = kNull;
    m_field.clear();
    m_log.clear();
    refreshHud(r.hud);
    setMessage(r.message);
}

void LevelScreen::doHint()
{
    if (m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Hint;
    ActionResult r = m_session.apply(act);
    m_renderer.setTarget(m_session.targetNodeId());     // gold ring on the target room
    refreshHud(r.hud);
    setMessage(r.message);
}

void LevelScreen::goToResult()
{
    LevelResult& res = m_ctx.result;
    res.stars = m_lastHud.stars;
    res.alarm = m_lastHud.alarm;
    res.optimal = m_lastHud.optimalSteps;
    res.penalties = m_lastHud.penalties;
    res.score = m_lastHud.score;
    res.won = (m_lastHud.status == HudStatus::Won);
    res.levelIndex = m_index;
    res.heapAct = false;
    res.sortedArray.clear();
    m_ctx.ui->requestScreen(ScreenId::Result);
}

// -------------------------------------------------------------------- input

void LevelScreen::updateTooltip(int mx, int my)
{
    float sx = (float)mx;
    float sy = (float)my;
    int id = m_renderer.pick(sx, sy);
    int key = 0;
    int height = 0;
    int size = 0;
    int bal = 0;
    int depth = 0;
    if (id != kNull && m_renderer.nodeInfo(id, key, height, size, bal, depth))
    {
        m_tooltip.showNode(sx + 16.f, sy + 16.f, key, height, bal, size, depth, false);
        return;
    }
    m_tooltip.hide();
}

void LevelScreen::onCanvasClick(int mx, int my)
{
    if (!m_ok || m_anim.busy())
    {
        return;
    }
    int id = m_renderer.pick((float)mx, (float)my);
    m_selected = id;
    m_renderer.setSelected(id);
    int key = 0;
    int h = 0;
    int s = 0;
    int b = 0;
    int d = 0;
    if (id != kNull && m_renderer.nodeInfo(id, key, h, s, b, d))
    {
        m_field.setValue(key);
    }
}

void LevelScreen::handleEvent(const sf::Event& e)
{
    // Esc is handled by UIManager.
    if (!m_ok)
    {
        return;
    }
    m_field.handleEvent(e);

    if (e.type == sf::Event::KeyPressed && !typing())
    {
        if (e.key.code == sf::Keyboard::Space)
        {
            if (m_anim.busy())
            {
                m_anim.togglePause();
                refreshStatus();
            }
            return;
        }
        if (e.key.code == sf::Keyboard::Right)
        {
            if (m_anim.busy())
            {
                m_anim.pause();
                m_anim.stepOne();
                refreshStatus();
            }
            return;
        }
        if (e.key.code == sf::Keyboard::Z)
        {
            doUndo();
            return;
        }
        if (e.key.code == sf::Keyboard::H)
        {
            doHint();
            return;
        }
    }

    m_toolbar.handleEvent(e);
    for (int i = 0; i < m_actCount; ++i)
    {
        m_act[i].handleEvent(e);
    }
    m_log.handleEvent(e);

    bool camera = m_renderer.handleCameraEvent(e);
    if (camera)
    {
        m_tooltip.hide();
    }
    else if (e.type == sf::Event::MouseMoved)
    {
        updateTooltip(e.mouseMove.x, e.mouseMove.y);
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        m_pressing = true;
        m_pressX = e.mouseButton.x;
        m_pressY = e.mouseButton.y;
    }
    else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
    {
        if (m_pressing)
        {
            float dx = (float)(e.mouseButton.x - m_pressX);
            float dy = (float)(e.mouseButton.y - m_pressY);
            if (dx < 0.f)
            {
                dx = -dx;
            }
            if (dy < 0.f)
            {
                dy = -dy;
            }
            if (dx < ClickSlop && dy < ClickSlop)
            {
                onCanvasClick(e.mouseButton.x, e.mouseButton.y);
            }
        }
        m_pressing = false;
    }
}

// ------------------------------------------------------------ update / draw

void LevelScreen::update(float dt)
{
    m_hud.update(dt);
    m_log.update(dt);
    m_toolbar.update(dt);
    if (!m_ok)
    {
        return;
    }
    m_field.update(dt);
    for (int i = 0; i < m_actCount; ++i)
    {
        m_act[i].update(dt);
    }
    m_renderer.update(dt);
    m_anim.setSpeed(m_toolbar.speed());
    m_anim.update(dt);

    if (m_endTimer > 0.f && !m_anim.busy())
    {
        m_endTimer -= dt;
        if (m_endTimer <= 0.f)
        {
            m_endTimer = 0.f;
            goToResult();
        }
    }
}

void LevelScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_canvasBox);

    if (!m_ok)
    {
        window.draw(m_notice);
        m_hud.draw(window);
        return;
    }

    m_renderer.draw(window);
    window.draw(m_hint);
    window.draw(m_msg);
    window.draw(m_bar);
    m_field.draw(window);
    window.draw(m_targetText);
    for (int i = 0; i < m_actCount; ++i)
    {
        m_act[i].draw(window);
    }

    m_hud.draw(window);
    m_log.draw(window);
    m_toolbar.draw(window);
    m_tooltip.draw(window);
}