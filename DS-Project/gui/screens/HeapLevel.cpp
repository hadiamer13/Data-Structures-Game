#include "HeapLevel.h"
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
    const int FirstHeapIndex = 15;   // level 4.1 ka campaign index, act 4 15 se shuru hota hai

    void styleText(sf::Text& t, const sf::Font& font, unsigned size, const sf::Color& col, float x, float y)
    {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setPosition(x, y);
    }

    // Greedy word wrap, character count ke hisaab se.
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

    // Pointer press aur release ke beech itna move hua ho to drag maana jaata hai.
    // True when the pointer moved far enough between press and release to be a drag.
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

    // Do index aapas mein parent aur child hain ya nahi.
    // Checks whether two heap indices are a parent and one of its children.
    bool isAdjacent(int a, int b)
    {
        if (a < 0 || b < 0 || a == b)
        {
            return false;
        }
        const bool aParentOfB = (b > 0) && ((b - 1) / 2 == a);
        const bool bParentOfA = (a > 0) && ((a - 1) / 2 == b);
        return aParentOfB || bParentOfA;
    }

    // Har mission ki aik instruction. Sirf text hai, is liye rang par depend nahi karta.
    // One instruction per mission. It is text only, so nothing depends on colour.
    const char* missionText(int mission)
    {
        if (mission == MissionKind::HeapBuild)
        {
            return "Activate the vault: swap a parent with a larger child, or select a node and press "
                "Heapify. Work bottom up from the last parent.";
        }
        if (mission == MissionKind::HeapRepair)
        {
            return "The root was overwritten. Restore the Max Heap property with swaps or Heapify.";
        }
        if (mission == MissionKind::HeapExtract)
        {
            return "Press Extract Max: the root moves to the sorted region and the heap shrinks by one.";
        }
        if (mission == MissionKind::HeapRace)
        {
            return "Build the heap, then extract every maximum before the operation limit runs out.";
        }
        return "Final vault: build the heap and extract every maximum to sort the array ascending.";
    }
}

//  construction
// Layout banata hai, level load karta hai, aur ready hone par buttons aur HUD set karta hai.
// Builds the layout, loads the level, and sets the buttons and HUD once it is ready.
HeapLevel::HeapLevel(AppContext& ctx)
    : m_ctx(ctx), m_index(FirstHeapIndex), m_ok(false), m_selected(kNull), m_endTimer(0.f),
      m_pressX(0), m_pressY(0), m_pressing(false), m_opCount(0),
      m_renderer(ctx.assets->font()), m_anim(&m_renderer)
{
    m_index = ctx.selectedLevel;
    if (m_index < FirstHeapIndex || m_index > 19)
    {
        m_index = FirstHeapIndex;
    }
    for (int i = 0; i < kMaxOps; ++i)
    {
        m_opCtx[i].owner = this;
        m_opCtx[i].op = 0;
    }

    buildLayout();
    m_anim.setCallbacks(&HeapLevel::onAnimEvent, &HeapLevel::onAnimDone, this);
    m_ok = loadLevel();

    if (m_ok)
    {
        m_renderer.snapTo(m_session.snapshot());
        m_renderer.fit(false);
        buildOpButtons();
        refreshPrompt();
        refreshHud(m_session.hud());
        m_log.addLine("Level ready. Click a node, then an adjacent node, to swap.");
        m_log.addLine("Wheel = zoom, drag background = pan, hover = info.");
    }
    else
    {
        m_hud.setTitle(std::string("Heap Fortress"));
        m_hud.setObjective(m_error);
        m_hud.setStatus("UNAVAILABLE");
        m_notice.setString(wrapChars(m_error, 60) + "\n\nPress Esc to go back.");
    }
}

// Canvas, bar, toolbar, log aur HUD ki positions set karta hai.
// Sets the positions of the canvas, bar, toolbar, log and HUD.
void HeapLevel::buildLayout()
{
    const sf::Font& font = m_ctx.assets->font();
    const float W = (float)Theme::WindowW;
    const float H = (float)Theme::WindowH;

    const sf::Texture& bg = m_ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / (float)bg.getSize().x, H / (float)bg.getSize().y);
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 190));

    const float cw = W - LogW;
    const float ch = H - HudH - ToolbarH - BarH;
    m_canvasBox.setPosition(0.f, HudH);
    m_canvasBox.setSize(sf::Vector2f(cw, ch));
    m_canvasBox.setFillColor(sf::Color(12, 6, 9, 170));
    m_canvasBox.setOutlineColor(Theme::BtnOutline);
    m_canvasBox.setOutlineThickness(-2.f);
    m_renderer.setCanvasRect(0.f, HudH, cw, ch);

    styleText(m_msg, font, 15, Theme::StarOn, 10.f, HudH + 8.f);
    styleText(m_prompt, font, 15, Theme::TextColor, 10.f, HudH + 30.f);
    styleText(m_hint, font, 12, sf::Color(170, 150, 150), 10.f, HudH + ch - 20.f);
    m_hint.setString("Wheel: zoom   Drag: pan   Click a node, then an adjacent node: swap   Z: undo   H: hint   Space: pause");
    styleText(m_notice, font, 24, Theme::TextColor, 40.f, HudH + 80.f);

    const float barY = H - ToolbarH - BarH;
    m_bar.setPosition(0.f, barY);
    m_bar.setSize(sf::Vector2f(cw, BarH));
    m_bar.setFillColor(Theme::TooltipBg);
    m_bar.setOutlineColor(Theme::BtnOutline);
    m_bar.setOutlineThickness(-2.f);
    styleText(m_selText, font, 14, Theme::TextColor, 16.f, barY + 24.f);

    m_hud.init(font, 0.f, 0.f, W, HudH);
    m_hud.setFrame(&m_ctx.assets->buttonFrame());
    m_log.init(font, W - LogW, HudH, LogW, H - HudH - ToolbarH);
    m_toolbar.init(font, 0.f, H - ToolbarH, W, ToolbarH);
    m_toolbar.setCallback(&HeapLevel::onToolbarClick, this);
    m_tooltip.init(font);
}

// Level levels/act4.json se aata hai. Index 15 se 19 tak slot 0 se 4 tak map hota hai.
// The level comes from levels/act4.json. Index 15 to 19 maps to slots 0 to 4.
bool HeapLevel::loadLevel()
{
    const int slot = m_index - FirstHeapIndex;
    LevelDefArray defs;
    std::string error;
    if (!LevelLoader::loadFile("levels/act4.json", defs, &error))
    {
        m_error = "Could not load levels/act4.json: " + error;
        return false;
    }
    if (defs.size() != 5)
    {
        m_error = "levels/act4.json must contain five levels.";
        return false;
    }
    m_def = defs[slot];
    if (!GameSession::isHeapMission(m_def.mission))
    {
        m_error = "Level \"" + m_def.name + "\" is not a heap mission.";
        return false;
    }
    if (!m_session.startLevel(m_def))
    {
        m_error = "The level data for \"" + m_def.name + "\" could not be started.";
        return false;
    }

    m_hud.setTitle("4." + std::to_string(slot + 1) + "  " + m_def.name);
    m_hud.setObjective(m_def.objective);
    return true;
}

//  mission rules
bool HeapLevel::allowsBuild() const
{
    return m_def.mission == MissionKind::HeapRace || m_def.mission == MissionKind::HeapBoss;
}

bool HeapLevel::allowsHeapify() const
{
    return m_def.mission != MissionKind::HeapExtract;
}

bool HeapLevel::allowsExtract() const
{
    return m_def.mission == MissionKind::HeapExtract ||
           m_def.mission == MissionKind::HeapRace || m_def.mission == MissionKind::HeapBoss;
}

bool HeapLevel::allowsSwap() const
{
    return m_def.mission != MissionKind::HeapExtract;
}

// Mission ke hisaab se sirf wahi buttons banate hain jo allowed hain.
// Builds only the buttons that the mission allows.
void HeapLevel::buildOpButtons()
{
    m_opCount = 0;
    m_opNextX = 170.f;   // "Selected:" label se door rakhne ke liye
    if (allowsBuild())
    {
        addOp(HeapOp::Build, "Build heap");
    }
    if (allowsHeapify())
    {
        addOp(HeapOp::Heapify, "Heapify selected");
    }
    if (allowsExtract())
    {
        addOp(HeapOp::Extract, "Extract max");
    }
    addOp(HeapOp::Clear, "Clear selection");
}

void HeapLevel::addOp(int op, const char* label)
{
    if (m_opCount >= kMaxOps)
    {
        return;
    }
    const sf::Font& font = m_ctx.assets->font();
    const float barY = (float)Theme::WindowH - ToolbarH - BarH;
    const float cy = barY + 34.f;

    m_opCtx[m_opCount].owner = this;
    m_opCtx[m_opCount].op = op;
    m_op[m_opCount].init(label, font, sf::Vector2f(0.f, cy), sf::Vector2f(120.f, 38.f));
    m_op[m_opCount].setTextSize(20);
    m_op[m_opCount].setPadding(12.f);
    const float w = m_op[m_opCount].width();
    m_op[m_opCount].setCenter(sf::Vector2f(m_opNextX + w * 0.5f, cy));
    m_op[m_opCount].setCallback(&HeapLevel::onOpClick, &m_opCtx[m_opCount]);
    m_opNextX += w + 14.f;
    m_opCount++;
}

//  callbacks
void HeapLevel::onToolbarClick(void* context, int action)
{
    static_cast<HeapLevel*>(context)->onToolbar(action);
}

void HeapLevel::onOpClick(void* context)
{
    OpCtx* c = static_cast<OpCtx*>(context);
    c->owner->onOp(c->op);
}

void HeapLevel::onAnimEvent(void* context, const Event& e, int index, int total)
{
    (void)index;
    (void)total;
    static_cast<HeapLevel*>(context)->animEvent(e);
}

void HeapLevel::onAnimDone(void* context)
{
    static_cast<HeapLevel*>(context)->animDone();
}

void HeapLevel::onToolbar(int action)
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

//  HUD
void HeapLevel::setMessage(const std::string& msg)
{
    m_msg.setString(wrapChars(msg, 100));
    if (!msg.empty())
    {
        m_log.addLine("> " + msg);
    }
}

void HeapLevel::setSelection(int id)
{
    m_selected = id;
    m_renderer.setSelected(id);
    refreshPrompt();
}

// Har number session ki HudState se aata hai, GUI maths se nahi.
// Every number comes from the session HudState, never from GUI maths.
void HeapLevel::refreshHud(const HudState& h)
{
    m_lastHud = h;
    m_hud.setAlarm(h.alarm, h.alarmCap);
    m_hud.setScore(h.score);
    m_hud.setSteps(h.stepsTaken, h.optimalSteps);
    refreshStatus();
}

void HeapLevel::refreshStatus()
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
        m_hud.setStatus("FAILED");
    }
    else
    {
        m_hud.setStatus("READY");
    }
}

void HeapLevel::refreshPrompt()
{
    std::string s = missionText(m_def.mission);
    s += " " + m_session.prompt();
    m_prompt.setString(wrapChars(s, 110));
    m_selText.setString(m_selected == kNull
                            ? std::string("Selected: none")
                            : ("Selected: index " + std::to_string(m_selected)));
}

// Tooltip mein node ki value, index aur depth dikhti hai.
// The tooltip shows the value, index and depth of a node.
void HeapLevel::updateTooltip(int mx, int my)
{
    const float sx = (float)mx;
    const float sy = (float)my;
    int id = m_renderer.pick(sx, sy);
    bool isCell = false;
    if (id == kNull)
    {
        id = m_renderer.pickCell(sx, sy);
        isCell = (id != kNull);
    }

    int key = 0;
    int height = 0;
    int size = 0;
    int balance = 0;
    int depth = 0;
    if (id != kNull && m_renderer.nodeInfo(id, key, height, size, balance, depth))
    {
        m_tooltip.showText(sx + 16.f, sy + 16.f,
                           std::string(isCell ? "array cell" : "heap node") +
                               "\nvalue: " + std::to_string(key) +
                               "\nindex: " + std::to_string(id) +
                               (isCell ? std::string("") : ("\ndepth: " + std::to_string(depth))));
        return;
    }
    m_tooltip.hide();
}

//  actions
// Click pehle index select karta hai. Select ho aur adjacent index par click ho to SwapHeap bhejta hai.
// Model direction aur heap rule check karta hai, is liye galat swap par alarm aur penalty lagti hai.
// The first click selects an index. A click on an adjacent index sends a SwapHeap.
// The model checks the direction and the heap rule, so a wrong swap costs alarm and a penalty.
void HeapLevel::onCanvasClick(int mx, int my)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    int id = m_renderer.pick((float)mx, (float)my);
    if (id == kNull)
    {
        id = m_renderer.pickCell((float)mx, (float)my);
    }
    if (id == kNull)
    {
        return;
    }

    if (m_selected != kNull && isAdjacent(m_selected, id) && allowsSwap())
    {
        PlayerAction act;
        act.kind = ActionKind::SwapHeap;
        act.a = m_selected;
        act.b = id;
        setSelection(kNull);
        submit(act);
        return;
    }
    setSelection(m_selected == id ? kNull : id);
}

void HeapLevel::onOp(int op)
{
    if (!m_ok || m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    if (op == HeapOp::Clear)
    {
        setSelection(kNull);
        setMessage("Selection cleared.");
        return;
    }

    PlayerAction act;
    if (op == HeapOp::Build)
    {
        act.kind = ActionKind::BuildHeap;
    }
    else if (op == HeapOp::Extract)
    {
        act.kind = ActionKind::ExtractMax;
    }
    else
    {
        if (m_selected == kNull)
        {
            setMessage("Select a node first, then press Heapify.");
            return;
        }
        act.kind = ActionKind::Heapify;
        act.a = m_selected;
    }
    setSelection(kNull);
    submit(act);
}

void HeapLevel::submit(const PlayerAction& act)
{
    m_renderer.setTarget(kNull);
    const ViewSnapshot before = m_session.snapshot();
    const ActionResult r = m_session.apply(act);
    setMessage(r.message);
    play(before, r);
}

// Har action aik batch hai. Events bina snapshot ke queue hote hain, aur aakhir mein
// final snapshot nodes aur array cells ko naye slots tak tween karta hai.
// DONE par koi animation nahi hoti.
// Each action is one batch. Events are queued without snapshots. The final snapshot
// tweens the nodes and array cells to their new slots at the end.
// DONE carries no animation.
void HeapLevel::play(const ViewSnapshot& before, const ActionResult& r)
{
    m_lastHud = r.hud;
    if (r.events.size() <= 1)        // sirf DONE (ya kuch nahi), matlab action ne kuch nahi badla
    {
        refreshHud(r.hud);
        refreshPrompt();
        return;
    }

    const ViewSnapshot after = m_session.snapshot();
    m_renderer.snapTo(before);
    m_anim.beginBatch(before);
    for (int i = 0; i < r.events.size(); ++i)
    {
        if (r.events[i].type == EventType::DONE)
        {
            continue;
        }
        m_anim.enqueue(r.events[i], nullptr);
    }
    m_log.addLine("--- play ---");
    m_anim.endBatch(after);
    refreshStatus();
}

void HeapLevel::animEvent(const Event& e)
{
    m_log.addLine(describeEventText(e));
    refreshStatus();
}

// Animation ke baad session ke authoritative numbers dikhte hain, aur level khatam ho to result ka timer chalta hai.
// After the animation, the authoritative session numbers are shown. If the level is over, the result timer starts.
void HeapLevel::animDone()
{
    m_renderer.setTarget(kNull);
    refreshHud(m_lastHud);
    refreshPrompt();
    if (m_lastHud.status != HudStatus::Playing)
    {
        m_log.addLine(m_lastHud.status == HudStatus::Won ? "LEVEL CLEARED" : "LEVEL FAILED");
        m_endTimer = 1.0f;            // Result screen animation ke baad hi aata hai
    }
}

// Undo ke baad hard sync hota hai, taake koi purani position na rahe.
// After undo the view is hard synced, so no old position is left behind.
void HeapLevel::doUndo()
{
    if (m_endTimer > 0.f || m_anim.busy())
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Undo;
    const ActionResult r = m_session.apply(act);
    m_anim.clear();
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setTarget(kNull);
    setSelection(kNull);
    refreshHud(r.hud);
    setMessage(r.message);
}

void HeapLevel::doReset()
{
    if (m_endTimer > 0.f)
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Reset;
    const ActionResult r = m_session.apply(act);
    m_anim.clear();
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setTarget(kNull);
    m_renderer.fit(true);
    m_log.clear();
    setSelection(kNull);
    refreshHud(r.hud);
    setMessage(r.message);
}

void HeapLevel::doHint()
{
    if (m_anim.busy() || m_endTimer > 0.f)
    {
        return;
    }
    PlayerAction act;
    act.kind = ActionKind::Hint;
    const ActionResult r = m_session.apply(act);
    m_renderer.setTarget(m_session.targetNodeId());   // jis index ko theek karna hai, uspar gold ring
    refreshHud(r.hud);
    setMessage(r.message);
}

// Result ko context mein bharta hai. Sorted array sirf tab dikhta hai jab sab extract ho chuke hon.
// Fills the shared result. The sorted array is shown only when every treasure is extracted in order.
void HeapLevel::goToResult()
{
    LevelResult& res = m_ctx.result;
    res.stars = m_lastHud.stars;
    res.alarm = m_lastHud.alarm;
    res.optimal = m_lastHud.optimalSteps;
    res.penalties = m_lastHud.penalties;
    res.score = m_lastHud.score;
    res.won = (m_lastHud.status == HudStatus::Won);
    res.levelIndex = m_index;
    res.heapAct = true;

    res.sortedArray.clear();
    const MaxHeap& heap = m_session.heap();
    if (res.won && heap.activeSize() == 0 && heap.isSortedAscending())
    {
        res.sortedArray = heap.array();
    }
    m_ctx.ui->requestScreen(ScreenId::Result);
}

//  input
// Keyboard shortcuts, toolbar, op buttons, camera aur mouse click sab yahin handle hote hain.
// Keyboard shortcuts, toolbar, op buttons, camera and mouse clicks are all handled here.
void HeapLevel::handleEvent(const sf::Event& e)
{
    if (!m_ok)
    {
        return;
    }

    if (e.type == sf::Event::KeyPressed)
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
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].handleEvent(e);
    }
    m_log.handleEvent(e);

    const bool camera = m_renderer.handleCameraEvent(e);
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
        if (m_pressing && !movedBeyondSlop(m_pressX, m_pressY, e.mouseButton.x, e.mouseButton.y))
        {
            onCanvasClick(e.mouseButton.x, e.mouseButton.y);
        }
        m_pressing = false;
    }
}

//  update and draw
void HeapLevel::update(float dt)
{
    m_hud.update(dt);
    m_log.update(dt);
    m_toolbar.update(dt);
    if (!m_ok)
    {
        return;
    }
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].update(dt);
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

void HeapLevel::draw(sf::RenderWindow& window)
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
    window.draw(m_prompt);
    window.draw(m_bar);
    window.draw(m_selText);
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].draw(window);
    }

    m_hud.draw(window);
    m_log.draw(window);
    m_toolbar.draw(window);
    m_tooltip.draw(window);
}