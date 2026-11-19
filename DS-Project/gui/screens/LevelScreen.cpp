// gui/screens/LevelScreen.cpp
#include "LevelScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../EventText.h"
#include "../../game/LevelLoader.h"
#include "../../contracts/Util.h"

namespace
{
    const float HudH = 84.f;
    const float ToolbarH = 48.f;
    const float LogW = 320.f;
    const float BarH = 64.f;
    const float ClickSlop = 5.f;

    bool structural(int type)
    {
        return type == EventType::INSERT || type == EventType::UNLINK || type == EventType::ROTATE;
    }

    // Label-only events: the snapshots already show the result, so they are not animated.
    bool animated(int type)
    {
        return type != EventType::UPDATE_HEIGHT && type != EventType::UPDATE_SIZE;
    }

    // True when the pointer moved far enough between press and release to count as a drag.
    bool movedBeyondSlop(int x0, int y0, int x1, int y1)
    {
        int dx = x1 - x0;
        int dy = y1 - y0;
        if (dx < 0)
        {
            dx = -dx;
        }
        if (dy < 0)
        {
            dy = -dy;
        }
        return (float)dx >= ClickSlop || (float)dy >= ClickSlop;
    }

    void styleText(sf::Text& t, const sf::Font& font, unsigned size, const sf::Color& col, float x, float y)
    {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setPosition(x, y);
    }

    // Greedy word wrap by character count.
    std::string wrapChars(const std::string& s, int width)
    {
        std::string out;
        std::string line;
        std::string word;
        for (size_t i = 0; i <= s.size(); ++i)
        {
            char c = (i < s.size()) ? s[i] : ' ';
            if (c != ' ')
            {
                word += c;
                continue;
            }
            if (word.empty())
            {
                continue;
            }
            if (!line.empty() && (int)(line.size() + 1 + word.size()) > width)
            {
                out += line + "\n";
                line = word;
            }
            else
            {
                line = line.empty() ? word : (line + " " + word);
            }
            word.clear();
        }
        out += line;
        return out;
    }

    const char* const TraversalNames[4] = { "PREORDER", "INORDER", "POSTORDER", "LEVEL ORDER" };
}

// ------------------------------------------------------------ construction

LevelScreen::LevelScreen(AppContext& ctx)
    : m_ctx(ctx), m_index(0), m_ok(false), m_mode(InputMode::Key), m_shownAlarm(0), m_endTimer(0.f),
    m_selected(kNull), m_dragToken(-1), m_dragX(0), m_dragY(0), m_dragPivot(kNull),
    m_renderer(ctx.assets->font()), m_anim(&m_renderer),
    m_actCount(0), m_pressX(0), m_pressY(0), m_pressing(false)
{
    m_index = ctx.selectedLevel;
    if (m_index < 0 || m_index > 19)
    {
        m_index = 0;
    }
    for (int i = 0; i < kMaxAct; ++i)
    {
        m_actCtx[i].owner = this;
        m_actCtx[i].which = i;
    }

    buildLayout();
    m_anim.setCallbacks(&LevelScreen::onAnimEvent, &LevelScreen::onAnimDone, this);
    m_renderer.setRingTexture(&ctx.assets->nodeRing());
    m_ok = loadLevel();

    if (m_ok)
    {
        m_renderer.snapTo(m_session.snapshot());
        m_renderer.fit(false);
        buildActionButtons();
        applyMissionMarks();
        refreshPrompt();
        refreshHud(m_session.hud());
        m_log.addLine("Level ready. Read the instruction line above the tree.");
        m_log.addLine("Wheel = zoom, drag background = pan, hover = info.");
    }
    else
    {
        m_hud.setTitle(m_def.name.empty() ? std::string("Level") : m_def.name);
        m_hud.setObjective(m_error);
        m_hud.setStatus("UNAVAILABLE");
        m_notice.setString(wrapChars(m_error, 60) + "\n\nPress Esc to go back.");
    }
}

// Which input scheme a mission uses.
int LevelScreen::modeFor(int mission)
{
    if (mission == MissionKind::Reconstruct)
    {
        return InputMode::Place;
    }
    if (mission == MissionKind::Traversal)
    {
        return InputMode::Sequence;
    }
    if (mission == MissionKind::HeightDiameter)
    {
        return InputMode::Numbers;
    }
    if (mission == MissionKind::LcaPath || mission == MissionKind::AvlBadges ||
        mission == MissionKind::AvlOrderStat || mission == MissionKind::BstRange)
    {
        return InputMode::Click;
    }
    if (mission == MissionKind::AvlSingle || mission == MissionKind::AvlDouble)
    {
        return InputMode::Pivot;
    }
    if (mission == MissionKind::Mirror || mission == MissionKind::AvlAuto)
    {
        return InputMode::Buttons;
    }
    return InputMode::Key;     // BST missions
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
    m_hint.setString("Wheel: zoom   Drag: pan   Space: pause   Z: undo   H: hint");
    styleText(m_msg, font, 15, Theme::StarOn, cx + 10.f, cy + 8.f);
    styleText(m_prompt, font, 15, Theme::TextColor, cx + 10.f, cy + 34.f);
    styleText(m_notice, font, 24, Theme::TextColor, cx + 40.f, cy + 80.f);

    float barY = H - ToolbarH - BarH;
    m_bar.setPosition(0.f, barY);
    m_bar.setSize(sf::Vector2f(cw, BarH));
    m_bar.setFillColor(Theme::TooltipBg);
    m_bar.setOutlineColor(Theme::BtnOutline);
    m_bar.setOutlineThickness(-2.f);

    m_field.init("KEY", font, 16.f, barY + 28.f, 130.f, 28.f);
    m_field.setCallback(&LevelScreen::onFieldClick, this);
    m_field2.init("DIAMETER", font, 170.f, barY + 28.f, 130.f, 28.f);
    m_field2.setCallback(&LevelScreen::onFieldClick, this);
    styleText(m_targetText, font, 18, Theme::StarOn, 165.f, barY + 22.f);

    // 1.2 token shelf, open-slot markers and the empty-canvas root drop box
    m_tokenCircle.setRadius(20.f);
    m_tokenCircle.setOrigin(20.f, 20.f);
    m_tokenCircle.setOutlineThickness(3.f);
    m_slotCircle.setPointCount(24);
    m_slotCircle.setFillColor(sf::Color::Transparent);
    m_slotCircle.setOutlineColor(sf::Color(170, 160, 160));
    m_slotCircle.setOutlineThickness(2.f);
    styleText(m_tokenText, font, 18, Theme::TextColor, 0.f, 0.f);
    styleText(m_slotText, font, 12, sf::Color(200, 190, 190), 0.f, 0.f);
    styleText(m_rootText, font, 20, Theme::TextColor, 0.f, 0.f);
    m_rootText.setString("Drop the first token here:\nit becomes the ROOT");
    {
        sf::FloatRect rb = m_rootText.getLocalBounds();
        m_rootText.setOrigin(rb.left + rb.width / 2.f, rb.top + rb.height / 2.f);
        m_rootText.setPosition(cx + cw / 2.f, cy + ch / 2.f);
    }
    m_rootBox.setSize(sf::Vector2f(380.f, 120.f));
    m_rootBox.setOrigin(190.f, 60.f);
    m_rootBox.setPosition(cx + cw / 2.f, cy + ch / 2.f);
    m_rootBox.setFillColor(sf::Color::Transparent);
    m_rootBox.setOutlineColor(sf::Color(170, 160, 160));
    m_rootBox.setOutlineThickness(3.f);

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
    const std::string id = std::to_string(act) + "." + std::to_string(slot + 1);

    if (GameSession::isHeapMission(m_def.mission))
    {
        m_error = "The Heap Fortress needs the HeapLevel screen (next step).";
        return false;
    }
    if (!GameSession::supportsMission(m_def.mission))
    {
        m_error = "Level " + id + " is not implemented yet.";
        return false;
    }
    if (!m_session.startLevel(m_def))
    {
        m_error = "The level data for \"" + m_def.name + "\" could not be started.";
        return false;
    }

    m_hud.setTitle(id + " " + m_def.name);
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
    const float barY = (float)Theme::WindowH - ToolbarH - BarH;
    const int mission = m_def.mission;
    m_mode = modeFor(mission);

    const char* labels[kMaxAct] = { "", "", "", "", "", "" };
    float w = 205.f;
    float step = 215.f;
    float x0 = 410.f;
    m_actCount = 0;

    if (m_mode == InputMode::Key)
    {
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
        m_hint.setString("Wheel: zoom   Drag: pan   Click a room: fills KEY   Space: pause   Z: undo   H: hint");
    }
    else if (m_mode == InputMode::Sequence)
    {
        labels[0] = "Submit route";
        labels[1] = "Clear picks";
        m_actCount = 2;
        w = 180.f;
        step = 190.f;
        x0 = 110.f;
        m_hint.setString("Wheel: zoom   Drag: pan   Click rooms in order   Z: remove last pick   H: hint");
    }
    else if (m_mode == InputMode::Numbers)
    {
        m_field.init("HEIGHT (rooms)", font, 16.f, barY + 28.f, 130.f, 28.f);
        m_field2.init("DIAMETER (edges)", font, 170.f, barY + 28.f, 130.f, 28.f);
        labels[0] = "Submit";
        m_actCount = 1;
        m_hint.setString("Wheel: zoom   Drag: pan   Type both numbers, then Submit (Enter)   H: hint");
    }
    else if (m_mode == InputMode::Click)
    {
        m_hint.setString("Wheel: zoom   Drag: pan   Click a room to answer   Z: undo   H: hint");
    }
    else if (m_mode == InputMode::Place)
    {
        m_hint.setString("Wheel: zoom   Drag: pan   Drag a token from the shelf onto the tree   Z: undo   H: hint");
    }
    else if (m_mode == InputMode::Pivot)
    {
        labels[0] = "Rotate LL";
        labels[1] = "Rotate RR";
        labels[2] = "Rotate LR";
        labels[3] = "Rotate RL";
        m_actCount = 4;
        w = 140.f;
        step = 150.f;
        x0 = 90.f;
        m_hint.setString("Wheel: zoom   Drag: pan   Drag the pivot onto a child, or click it and press a rotation   H: hint");
    }
    else if (mission == MissionKind::Mirror)
    {
        labels[0] = "Mirror";
        m_actCount = 1;
        w = 160.f;
        x0 = 110.f;
        m_hint.setString("Wheel: zoom   Drag: pan   Mirror swaps every room's children   Z: undo");
    }
    else
    {
        labels[0] = "None";
        labels[1] = "LL";
        labels[2] = "RR";
        labels[3] = "LR";
        labels[4] = "RL";
        m_actCount = 5;
        w = 110.f;
        step = 120.f;
        x0 = 80.f;
        m_hint.setString("Wheel: zoom   Drag: pan   Pick the rotation the next insert will cause   H: hint");
    }

    for (int i = 0; i < m_actCount; ++i)
    {
        float cx = x0 + (float)i * step;
        m_act[i].init(labels[i], font, sf::Vector2f(cx, barY + 34.f), sf::Vector2f(w, 38.f));
        m_act[i].setCallback(&LevelScreen::onActionClick, &m_actCtx[i]);
        m_act[i].setFrame(&m_ctx.assets->buttonFrame());
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
    if (m_def.mission == MissionKind::BstDelete)
    {
        setMessage("Pick the delete case with a button.");
    }
    else
    {
        doAction(0);       // Enter = the single / first action
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
    m_msg.setString(wrapChars(msg, 100));
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

bool LevelScreen::fieldsActive() const
{
    return m_mode == InputMode::Key || m_mode == InputMode::Numbers;
}

bool LevelScreen::typing() const
{
    return fieldsActive() && (m_field.focused() || m_field2.focused());
}

// Key (value) of a room id as text, "?" if unknown.
std::string LevelScreen::keyOf(int id) const
{
    int key = 0;
    int h = 0;
    int s = 0;
    int b = 0;
    int d = 0;
    if (id != kNull && m_renderer.nodeInfo(id, key, h, s, b, d))
    {
        return std::to_string(key);
    }
    return "?";
}

// One instruction line under the message line. Text only, no colour needed.
void LevelScreen::refreshPrompt()
{
    if (m_mode == InputMode::Place)
    {
        m_tokens = m_session.reconstructTokens();
    }
    std::string s;
    const int mission = m_def.mission;

    if (mission == MissionKind::Traversal)
    {
        const int type = (m_def.param >= 0 && m_def.param <= 3) ? m_def.param : 0;
        s = std::string("Click the rooms in ") + TraversalNames[type] + " order. Picked " +
            std::to_string(m_clicks.size()) + " of " + std::to_string(m_session.binaryTree().nodeCount()) + ":";
        for (int i = 0; i < m_clicks.size(); ++i)
        {
            s += " " + keyOf(m_clicks[i]);
        }
    }
    else if (mission == MissionKind::HeightDiameter)
    {
        s = "Enter the height (rooms on the longest root-to-leaf path) and the diameter "
            "(edges on the longest path between any two rooms), then Submit.";
    }
    else if (mission == MissionKind::LcaPath)
    {
        s = "Routes start at rooms " + keyOf(m_session.lcaTargetA()) + " and " + keyOf(m_session.lcaTargetB()) +
            " (gold rings). Click the room where the two routes meet.";
    }
    else if (mission == MissionKind::Reconstruct)
    {
        s = "Drag each token onto the tree: the first is the root, then drop on the left or right side "
            "of a placed room. " + m_session.reconstructClue();
    }
    else if (mission == MissionKind::Mirror)
    {
        s = "Press Mirror to swap the children of every room. Pressing it again mirrors back.";
    }
    else if (mission == MissionKind::BstRange)
    {
        s = m_session.prompt();
    }
    else if (m_mode == InputMode::Pivot)
    {
        s = m_session.prompt();
        s += (m_selected == kNull) ? " No pivot picked." : (" Pivot: room " + keyOf(m_selected) + ".");
        s += " Drag the pivot onto a child, or pick a rotation button.";
    }
    else if (m_mode != InputMode::Key)
    {
        s = m_session.prompt();     // Act 3 prompts come from the session
    }
    m_prompt.setString(wrapChars(s, 95));
}

// Marks that belong to the mission (not to an animation). Re-applied after every snapTo.
void LevelScreen::applyMissionMarks()
{
    if (m_def.mission == MissionKind::LcaPath)
    {
        m_renderer.setMark(m_session.lcaTargetA(), NodeMark::Result);
        m_renderer.setMark(m_session.lcaTargetB(), NodeMark::Result);
    }
    for (int i = 0; i < m_clicks.size(); ++i)
    {
        m_renderer.setMark(m_clicks[i], NodeMark::Visit);
    }
}

// ------------------------------------------------------------------ actions

void LevelScreen::doAction(int which)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;      // input is locked while the animation plays
    }
    const int mission = m_def.mission;
    PlayerAction act;

    if (m_mode == InputMode::Key)
    {
        if (!m_field.hasValue())
        {
            setMessage("Type a key in the KEY box first.");
            return;
        }
        act.a = m_field.value();
        if (mission == MissionKind::BstDelete)
        {
            act.kind = ActionKind::Delete;
            act.c = (which == 0) ? DeleteCase::Leaf : ((which == 1) ? DeleteCase::OneChild : DeleteCase::TwoChildren);
        }
        else if (mission == MissionKind::BstInsert)
        {
            act.kind = ActionKind::Insert;
        }
        else
        {
            act.kind = ActionKind::Search;
        }
    }
    else if (m_mode == InputMode::Sequence)
    {
        if (which == 1)
        {
            m_clicks.clear();
            m_renderer.clearHighlights();
            applyMissionMarks();
            refreshPrompt();
            return;
        }
        if (m_clicks.size() == 0)
        {
            setMessage("Click the rooms first.");
            return;
        }
        act.kind = ActionKind::SubmitSequence;
        act.seq = m_clicks;
        m_clicks.clear();
    }
    else if (m_mode == InputMode::Numbers)
    {
        if (!m_field.hasValue() || !m_field2.hasValue())
        {
            setMessage("Type both the height and the diameter.");
            return;
        }
        act.kind = ActionKind::SubmitSequence;
        act.a = m_field.value();
        act.b = m_field2.value();
    }
    else if (m_mode == InputMode::Pivot)
    {
        if (m_selected == kNull)
        {
            setMessage("Click a room first: it is the pivot.");
            return;
        }
        const int types[4] = { RotationType::LL, RotationType::RR, RotationType::LR, RotationType::RL };
        act.kind = ActionKind::Rotate;
        act.a = m_selected;
        act.b = types[which < 4 ? which : 0];
    }
    else if (mission == MissionKind::Mirror)
    {
        act.kind = ActionKind::Rotate;
    }
    else if (mission == MissionKind::AvlAuto)
    {
        const int types[5] = { RotationType::None, RotationType::LL, RotationType::RR,
            RotationType::LR, RotationType::RL };
        act.kind = ActionKind::Rotate;
        act.b = types[which < 5 ? which : 0];
    }
    else
    {
        return;      // Click mode has no buttons
    }

    submit(act);
    if (m_mode == InputMode::Pivot)
    {
        m_selected = kNull;
        m_renderer.setSelected(kNull);
    }
}

// Sends one action to the session and plays the resulting events.
void LevelScreen::submit(const PlayerAction& act)
{
    m_renderer.setTarget(kNull);
    ViewSnapshot before = m_session.snapshot();
    ActionResult r = m_session.apply(act);
    setMessage(r.message);
    play(before, r);
}

// One batch. The session returns the final state; the last structural event
// (INSERT / UNLINK / ROTATE) carries the "after" snapshot so the tree tweens when it plays.
// Events that only update labels are dropped (the snapshots show the result).
void LevelScreen::play(const ViewSnapshot& before, const ActionResult& r)
{
    if (r.events.size() == 0)
    {
        refreshHud(r.hud);      // rejected action: nothing to animate
        refreshPrompt();
        return;
    }

    ViewSnapshot after = m_session.snapshot();
    m_lastHud = r.hud;
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
        if (animated(r.events[i].type) && structural(r.events[i].type))
        {
            lastStruct = i;
        }
    }

    m_renderer.snapTo(before);
    applyMissionMarks();
    m_anim.beginBatch(before);
    for (int i = 0; i < r.events.size(); ++i)
    {
        if (!animated(r.events[i].type))
        {
            continue;
        }
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
    applyMissionMarks();
    refreshPrompt();
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
    if (m_mode == InputMode::Sequence && m_clicks.size() > 0)
    {
        m_clicks.pop();        // 1.1: Z removes the last pick (no model state yet)
        m_renderer.clearHighlights();
        applyMissionMarks();
        refreshPrompt();
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
    applyMissionMarks();
    refreshHud(r.hud);
    refreshPrompt();
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
    m_clicks.clear();
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setTarget(kNull);
    m_renderer.setSelected(kNull);
    m_renderer.fit(true);
    m_selected = kNull;
    m_field.clear();
    m_field2.clear();
    m_log.clear();
    applyMissionMarks();
    refreshHud(r.hud);
    refreshPrompt();
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
        if (m_def.mission == MissionKind::HeightDiameter)
        {
            // Level 1.3 asks for the height: do not give it away.
            m_tooltip.showText(sx + 16.f, sy + 16.f,
                "key: " + std::to_string(key) + "\ndepth: " + std::to_string(depth));
        }
        else
        {
            m_tooltip.showNode(sx + 16.f, sy + 16.f, key, height, bal, size, depth,
                m_renderer.kind() == TreeKind::AVL);
        }
        return;
    }
    m_tooltip.hide();
}

void LevelScreen::onCanvasClick(int mx, int my)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    int id = m_renderer.pick((float)mx, (float)my);

    if (m_mode == InputMode::Key)
    {
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
        return;
    }
    if (id == kNull)
    {
        return;
    }

    if (m_mode == InputMode::Sequence)
    {
        if (m_clicks.indexOf(id) != -1)
        {
            setMessage("That room is already picked. Press Z to remove the last pick.");
            return;
        }
        m_clicks.push(id);
        m_renderer.setMark(id, NodeMark::Visit);
        refreshPrompt();
        if (m_clicks.size() >= m_session.binaryTree().nodeCount())
        {
            doAction(0);       // last room picked: submit automatically
        }
    }
    else if (m_mode == InputMode::Click)
    {
        PlayerAction act;
        act.kind = ActionKind::ClickNode;
        act.a = id;
        submit(act);
    }
    else if (m_mode == InputMode::Pivot)
    {
        m_selected = id;
        m_renderer.setSelected(id);
        refreshPrompt();
    }
}

void LevelScreen::handleEvent(const sf::Event& e)
{
    // Esc is handled by UIManager.
    if (!m_ok)
    {
        return;
    }
    if (fieldsActive())
    {
        m_field.handleEvent(e);
        if (m_mode == InputMode::Numbers)
        {
            m_field2.handleEvent(e);
        }
    }

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

    if (m_mode == InputMode::Place)
    {
        handleTokenDrag(e);
    }

    // Pivot drag (3.2 / 3.3): follow the pointer while a pivot room is held.
    if (e.type == sf::Event::MouseMoved && m_dragPivot != kNull)
    {
        m_dragX = e.mouseMove.x;
        m_dragY = e.mouseMove.y;
    }

    bool camera = m_renderer.handleCameraEvent(e);
    if (camera)
    {
        m_tooltip.hide();
    }
    else if (e.type == sf::Event::MouseMoved && m_dragToken == -1)
    {
        updateTooltip(e.mouseMove.x, e.mouseMove.y);
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        m_pressing = true;
        m_pressX = e.mouseButton.x;
        m_pressY = e.mouseButton.y;
        m_dragX = e.mouseButton.x;
        m_dragY = e.mouseButton.y;
        m_dragPivot = kNull;
        if (m_mode == InputMode::Pivot && !m_anim.busy() && m_endTimer <= 0.f)
        {
            m_dragPivot = m_renderer.pick((float)m_pressX, (float)m_pressY);
            if (m_dragPivot != kNull)
            {
                m_selected = m_dragPivot;
                m_renderer.setSelected(m_dragPivot);
                m_tooltip.hide();
            }
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
    {
        if (m_pressing)
        {
            const bool moved = movedBeyondSlop(m_pressX, m_pressY, e.mouseButton.x, e.mouseButton.y);
            if (m_dragPivot != kNull && moved)
            {
                dropPivot(m_dragPivot, e.mouseButton.x, e.mouseButton.y);
            }
            if (!moved)
            {
                onCanvasClick(e.mouseButton.x, e.mouseButton.y);
            }
        }
        m_dragPivot = kNull;
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
    if (fieldsActive())
    {
        m_field.update(dt);
        if (m_mode == InputMode::Numbers)
        {
            m_field2.update(dt);
        }
    }
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
    drawSlotMarkers(window);
    window.draw(m_hint);
    window.draw(m_msg);
    window.draw(m_prompt);
    window.draw(m_bar);
    if (m_mode == InputMode::Key)
    {
        m_field.draw(window);
        window.draw(m_targetText);
    }
    else if (m_mode == InputMode::Numbers)
    {
        m_field.draw(window);
        m_field2.draw(window);
    }
    else if (m_mode == InputMode::Place)
    {
        drawTokens(window);
    }
    for (int i = 0; i < m_actCount; ++i)
    {
        m_act[i].draw(window);
    }

    m_hud.draw(window);
    m_log.draw(window);
    m_toolbar.draw(window);
    m_tooltip.draw(window);
    drawDragged(window);
    drawPivotDrag(window);
}

// ------------------------------------------------------ 3.2 / 3.3 pivot drag

// Dropping the pivot on its left child means LL (single) or LR (double);
// dropping it on its right child means RR (single) or RL (double).
// Anything else is rejected here, before the model, with a plain message.
void LevelScreen::dropPivot(int pivot, int mx, int my)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    const ViewSnapshot snap = m_session.snapshot();
    if (pivot < 0 || pivot >= snap.nodes.size())
    {
        return;
    }
    const ViewNode& node = snap.nodes[pivot];
    const int target = m_renderer.pick((float)mx, (float)my);

    const bool single = (m_def.mission == MissionKind::AvlSingle);
    int type = RotationType::None;
    if (target != kNull && target != pivot)
    {
        if (target == node.left)
        {
            type = single ? RotationType::LL : RotationType::LR;
        }
        else if (target == node.right)
        {
            type = single ? RotationType::RR : RotationType::RL;
        }
    }
    if (type == RotationType::None)
    {
        setMessage("Drag the pivot onto one of its children.");
        return;
    }

    PlayerAction act;
    act.kind = ActionKind::Rotate;
    act.a = pivot;
    act.b = type;
    submit(act);
    m_selected = kNull;
    m_renderer.setSelected(kNull);
}

// Line from the held pivot to the pointer, so the drag is visible.
void LevelScreen::drawPivotDrag(sf::RenderWindow& window)
{
    if (m_dragPivot == kNull)
    {
        return;
    }
    float sx = 0.f;
    float sy = 0.f;
    if (!m_renderer.nodeScreenPos(m_dragPivot, sx, sy))
    {
        return;
    }
    sf::Vertex line[2];
    line[0] = sf::Vertex(sf::Vector2f(sx, sy), Theme::StarOn);
    line[1] = sf::Vertex(sf::Vector2f((float)m_dragX, (float)m_dragY), Theme::StarOn);
    window.draw(line, 2, sf::Lines);
}

// ------------------------------------------------------ 1.2 token placement

namespace
{
    const float TokenX0 = 40.f;
    const float TokenStep = 52.f;
    const float TokenR = 22.f;

    void centreText(sf::Text& t, float x, float y)
    {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        t.setPosition(x, y);
    }
}

// Shelf index under the pointer, -1 if none.
int LevelScreen::tokenAt(int mx, int my) const
{
    const float cy = (float)Theme::WindowH - ToolbarH - BarH + 32.f;
    for (int i = 0; i < m_tokens.size(); ++i)
    {
        const float cx = TokenX0 + (float)i * TokenStep;
        if (cx > (float)Theme::WindowW - LogW - 20.f)
        {
            break;
        }
        const float dx = (float)mx - cx;
        const float dy = (float)my - cy;
        if (dx * dx + dy * dy <= TokenR * TokenR)
        {
            return i;
        }
    }
    return -1;
}

void LevelScreen::handleTokenDrag(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
    {
        if (m_anim.busy() || m_endTimer > 0.f)
        {
            return;
        }
        const int t = tokenAt(e.mouseButton.x, e.mouseButton.y);
        if (t != -1)
        {
            m_dragToken = t;
            m_dragX = e.mouseButton.x;
            m_dragY = e.mouseButton.y;
            m_tooltip.hide();
        }
    }
    else if (e.type == sf::Event::MouseMoved && m_dragToken != -1)
    {
        m_dragX = e.mouseMove.x;
        m_dragY = e.mouseMove.y;
        m_renderer.setSelected(m_renderer.pick((float)m_dragX, (float)m_dragY));   // ring the room under the pointer
    }
    else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left && m_dragToken != -1)
    {
        const int token = m_dragToken;
        m_dragToken = -1;
        m_renderer.setSelected(kNull);
        dropToken(token, e.mouseButton.x, e.mouseButton.y);
    }
}

// Release point -> Place action. The model decides whether the placement is right.
void LevelScreen::dropToken(int token, int mx, int my)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f || token < 0 || token >= m_tokens.size())
    {
        return;
    }
    const float canvasR = (float)Theme::WindowW - LogW;
    const float canvasB = (float)Theme::WindowH - ToolbarH - BarH;
    if ((float)mx < 0.f || (float)mx > canvasR || (float)my < HudH || (float)my > canvasB)
    {
        return;      // dropped outside the canvas: token stays on the shelf
    }

    const BinaryTree& built = m_session.builtTree();
    PlayerAction act;
    act.kind = ActionKind::Place;
    act.a = m_tokens[token];
    if (built.root() == nullptr)
    {
        act.b = kNull;
        act.c = Side::None;
    }
    else
    {
        const int id = m_renderer.pick((float)mx, (float)my);
        float sx = 0.f;
        float sy = 0.f;
        if (id == kNull || id >= built.pool().size() || !m_renderer.nodeScreenPos(id, sx, sy))
        {
            setMessage("Drop the token on a placed room (left or right side of it).");
            return;
        }
        const TreeNode* n = built.pool()[id];
        int side = ((float)mx < sx) ? Side::Left : Side::Right;
        if (side == Side::Left && n->left != nullptr && n->right == nullptr)
        {
            side = Side::Right;      // only one side is free: use it
        }
        else if (side == Side::Right && n->right != nullptr && n->left == nullptr)
        {
            side = Side::Left;
        }
        act.b = id;
        act.c = side;
    }
    submit(act);
}

void LevelScreen::drawTokens(sf::RenderWindow& window)
{
    const float cy = (float)Theme::WindowH - ToolbarH - BarH + 32.f;
    if (m_tokens.size() == 0)
    {
        m_tokenText.setString("All tokens placed");
        m_tokenText.setOrigin(0.f, 0.f);
        m_tokenText.setPosition(16.f, cy - 12.f);
        window.draw(m_tokenText);
        return;
    }
    for (int i = 0; i < m_tokens.size(); ++i)
    {
        const float cx = TokenX0 + (float)i * TokenStep;
        if (cx > (float)Theme::WindowW - LogW - 20.f)
        {
            break;
        }
        const bool lifted = (i == m_dragToken);
        m_tokenCircle.setPosition(cx, cy);
        m_tokenCircle.setFillColor(lifted ? sf::Color(40, 30, 34, 90) : Theme::NodeUnlocked);
        m_tokenCircle.setOutlineColor(lifted ? sf::Color(120, 110, 115, 120) : Theme::NodeGlow);
        window.draw(m_tokenCircle);
        if (!lifted)
        {
            m_tokenText.setString(std::to_string(m_tokens[i]));
            centreText(m_tokenText, cx, cy);
            window.draw(m_tokenText);
        }
    }
}

void LevelScreen::drawDragged(sf::RenderWindow& window)
{
    if (m_mode != InputMode::Place || m_dragToken < 0 || m_dragToken >= m_tokens.size())
    {
        return;
    }
    m_tokenCircle.setPosition((float)m_dragX, (float)m_dragY);
    m_tokenCircle.setFillColor(Theme::NodeDone);
    m_tokenCircle.setOutlineColor(Theme::StarOn);
    window.draw(m_tokenCircle);
    m_tokenText.setString(std::to_string(m_tokens[m_dragToken]));
    centreText(m_tokenText, (float)m_dragX, (float)m_dragY);
    window.draw(m_tokenText);
}

// Open child slots next to every placed room. The letter L / R is the signal, not the colour.
void LevelScreen::drawSlotMarkers(sf::RenderWindow& window)
{
    if (m_mode != InputMode::Place)
    {
        return;
    }
    const BinaryTree& built = m_session.builtTree();
    if (built.root() == nullptr)
    {
        window.draw(m_rootBox);
        window.draw(m_rootText);
        return;
    }
    if (m_anim.busy())
    {
        return;
    }
    const float z = m_renderer.zoom();
    const float r = maxF(7.f, 10.f * z);
    const float canvasR = (float)Theme::WindowW - LogW;
    const float canvasB = (float)Theme::WindowH - ToolbarH - BarH;
    for (int i = 0; i < built.pool().size(); ++i)
    {
        float sx = 0.f;
        float sy = 0.f;
        if (!built.isAlive(i) || !m_renderer.nodeScreenPos(i, sx, sy))
        {
            continue;
        }
        const TreeNode* n = built.pool()[i];
        for (int side = 0; side < 2; ++side)
        {
            if ((side == 0 ? n->left : n->right) != nullptr)
            {
                continue;
            }
            const float px = sx + (side == 0 ? -28.f : 28.f) * z;
            const float py = sy + 42.f * z;
            if (px < 8.f || px > canvasR - 8.f || py < HudH + 8.f || py > canvasB - 8.f)
            {
                continue;
            }
            m_slotCircle.setRadius(r);
            m_slotCircle.setOrigin(r, r);
            m_slotCircle.setPosition(px, py);
            window.draw(m_slotCircle);
            if (z >= 0.4f)
            {
                m_slotText.setString(side == 0 ? "L" : "R");
                centreText(m_slotText, px, py);
                window.draw(m_slotText);
            }
        }
    }
}