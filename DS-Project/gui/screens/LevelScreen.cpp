// gui/screens/LevelScreen.cpp
#include "LevelScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../FakeCampaignData.h"   // INTEGRATION: remove (LevelDef supplies name/objective)
#include "../FakeSnapshots.h"      // INTEGRATION: remove (SnapshotBuilder supplies snapshots)

namespace
{
    // Layout (window 1280 x 720)
    const float HudH = 84.f;
    const float ToolbarH = 48.f;
    const float LogW = 320.f;

    const float ClickSlop = 5.f;

    float evDuration(int type)
    {
        if (type == EventType::VISIT || type == EventType::COMPARE)
        {
            return 0.5f;
        }
        if (type == EventType::ROTATE)
        {
            return 1.0f;
        }
        if (type == EventType::HEAP_SWAP || type == EventType::SWAP_VALUE)
        {
            return 0.7f;
        }
        if (type == EventType::DONE)
        {
            return 0.05f;
        }
        return 0.4f;
    }

    Event makeEv(int type, int id, int cost)
    {
        Event e(type);
        e.id = id;
        e.alarmCost = cost;
        return e;
    }

    Event makeHeapEv(int type, int idx, int idx2, int cost)
    {
        Event e(type);
        e.idx = idx;
        e.idx2 = idx2;
        e.alarmCost = cost;
        return e;
    }

    const char* rotationName(int r)
    {
        if (r == RotationType::LL)
        {
            return "LL (right rotation)";
        }
        if (r == RotationType::RR)
        {
            return "RR (left rotation)";
        }
        if (r == RotationType::LR)
        {
            return "LR (double)";
        }
        if (r == RotationType::RL)
        {
            return "RL (double)";
        }
        return "?";
    }

    const char* swapReason(int r)
    {
        if (r == SwapReason::SiftUp)
        {
            return "sift-up";
        }
        if (r == SwapReason::SiftDown)
        {
            return "sift-down";
        }
        if (r == SwapReason::Extract)
        {
            return "extract";
        }
        if (r == SwapReason::Player)
        {
            return "player";
        }
        return "build";
    }

    // INTEGRATION: replace with EventLog::describe(e) from the model.
    std::string describeEvent(const Event& e)
    {
        std::string s;
        int t = e.type;
        if (t == EventType::VISIT)
        {
            s = "Visit room #" + std::to_string(e.id);
        }
        else if (t == EventType::COMPARE)
        {
            const char* dir = (e.result == CompareResult::Less) ? "go left"
                : ((e.result == CompareResult::Greater) ? "go right" : "found");
            s = "Compare " + std::to_string(e.key) + " with room #" + std::to_string(e.id) + ": " + dir;
        }
        else if (t == EventType::INSERT)
        {
            s = "Insert room #" + std::to_string(e.id) + " under #" + std::to_string(e.id2);
        }
        else if (t == EventType::UNLINK)
        {
            s = "Unlink room #" + std::to_string(e.id);
        }
        else if (t == EventType::SWAP_VALUE)
        {
            s = "Swap values of rooms #" + std::to_string(e.id) + " and #" + std::to_string(e.id2);
        }
        else if (t == EventType::ROTATE)
        {
            s = std::string("Rotate ") + rotationName(e.result) + " at pivot #" + std::to_string(e.id);
        }
        else if (t == EventType::UPDATE_HEIGHT)
        {
            s = "Height of #" + std::to_string(e.id) + ": " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
        }
        else if (t == EventType::UPDATE_SIZE)
        {
            s = "Size of #" + std::to_string(e.id) + ": " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
        }
        else if (t == EventType::BALANCE_ALERT)
        {
            s = "BALANCE ALERT at #" + std::to_string(e.id) + " (bf " + std::to_string(e.newV) + ")";
        }
        else if (t == EventType::HIGHLIGHT_SUBTREE)
        {
            s = std::string((e.result == HighlightMode::Skipped) ? "Skip" : "Mark")
                + " subtree at #" + std::to_string(e.id);
        }
        else if (t == EventType::HEAP_BUILD)
        {
            s = "Build heap, array size " + std::to_string(e.newV);
        }
        else if (t == EventType::HEAP_SWAP)
        {
            s = "Swap cells [" + std::to_string(e.idx) + "] and [" + std::to_string(e.idx2) + "] ("
                + swapReason(e.result) + ")";
        }
        else if (t == EventType::HEAPIFY_START)
        {
            s = "Heapify from [" + std::to_string(e.idx) + "], heap size " + std::to_string(e.newV);
        }
        else if (t == EventType::HEAPIFY_DONE)
        {
            s = "Heapify done: heap property restored";
        }
        else if (t == EventType::EXTRACT_MAX)
        {
            s = "Extract max " + std::to_string(e.key) + " (heap size " + std::to_string(e.newV) + ")";
        }
        else if (t == EventType::MARK_SORTED)
        {
            s = "Value " + std::to_string(e.key) + " moved to sorted region";
        }
        else if (t == EventType::HEAP_SIZE_CHANGE)
        {
            s = "Heap size " + std::to_string(e.oldV) + " -> " + std::to_string(e.newV);
        }
        else if (t == EventType::HEAP_SORT_COMPLETE)
        {
            s = "HEAP SORT COMPLETE";
        }
        else if (t == EventType::DONE)
        {
            s = "Done";
        }
        else
        {
            s = "Event " + std::to_string(t);
        }
        return s;
    }
}

// ------------------------------------------------------------ construction

LevelScreen::LevelScreen(AppContext& ctx)
    : m_ctx(ctx), m_index(0),
    m_renderer(ctx.assets->font()),
    m_pressX(0), m_pressY(0), m_pressing(false),
    m_qCount(0), m_qIndex(0),
    m_busy(false), m_paused(false), m_evActive(false), m_stepping(false), m_stepReq(false),
    m_done(false), m_evT(0.f), m_speed(1.f),
    m_targetId(kNull), m_alarm(0), m_alarmCap(0), m_optimal(0)
{
    m_index = ctx.selectedLevel;
    if (m_index < 0 || m_index > 19)
    {
        m_index = 0;
    }

    buildLayout();
    loadFakeScenario();

    // INTEGRATION: title/objective come from LevelDef.
    m_hud.setTitle(FakeCampaign::Names[m_index]);
    m_hud.setObjective(FakeCampaign::Objectives[m_index]);

    m_renderer.snapTo(m_start);
    m_log.addLine("Level ready. Press Space (or Replay) to play the demo.");
    m_log.addLine("Wheel = zoom, drag background = pan, hover = info.");
    refreshHud();
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
    float ch = H - HudH - ToolbarH;

    m_canvasBox.setPosition(cx, cy);
    m_canvasBox.setSize(sf::Vector2f(cw, ch));
    m_canvasBox.setFillColor(sf::Color(12, 6, 9, 170));
    m_canvasBox.setOutlineColor(Theme::BtnOutline);
    m_canvasBox.setOutlineThickness(-2.f);

    m_canvasHint.setFont(font);
    m_canvasHint.setCharacterSize(14);
    m_canvasHint.setFillColor(sf::Color(170, 150, 150));
    m_canvasHint.setString("Wheel: zoom   Drag background: pan   Space: play/pause   Right: step   Z: undo   H: hint");
    m_canvasHint.setPosition(cx + 10.f, cy + ch - 24.f);

    m_renderer.setCanvasRect(cx, cy, cw, ch);

    m_hud.init(font, 0.f, 0.f, W, HudH);
    m_log.init(font, W - LogW, HudH, LogW, H - HudH - ToolbarH);
    m_toolbar.init(font, 0.f, H - ToolbarH, W, ToolbarH);
    m_toolbar.setCallback(&LevelScreen::onToolbarClick, this);
    m_tooltip.init(font);
}

// ------------------------------------------------------------ FAKE DRIVER

void LevelScreen::addEvent(const Event& e)
{
    if (m_qCount < kMaxQueue)
    {
        m_queue[m_qCount] = e;
        m_qCount++;
    }
}

// INTEGRATION: delete. Real levels come from GameSession::startLevel(LevelDef).
void LevelScreen::loadFakeScenario()
{
    int act = m_index / 5;
    m_qCount = 0;
    m_qIndex = 0;

    if (act == 2)
    {
        // Act 3 style: LL imbalance at the root, rotateRight at 50, pivot = 30 (id 1).
        m_start = FakeSnapshots::avlLLImbalance();
        m_final = FakeSnapshots::avlLLAfterRotation();
        m_targetId = 0;
        m_optimal = 3;

        addEvent(makeEv(EventType::VISIT, 0, 1));
        addEvent(makeEv(EventType::VISIT, 1, 1));
        addEvent(makeEv(EventType::BALANCE_ALERT, 0, 0));
        m_queue[m_qCount - 1].newV = 2;

        Event rot = makeEv(EventType::ROTATE, 1, 0);
        rot.result = RotationType::LL;
        addEvent(rot);

        Event h0 = makeEv(EventType::UPDATE_HEIGHT, 0, 0);
        h0.oldV = 4;
        h0.newV = 2;
        addEvent(h0);
        Event h1 = makeEv(EventType::UPDATE_HEIGHT, 1, 0);
        h1.oldV = 3;
        h1.newV = 3;
        addEvent(h1);
        addEvent(makeEv(EventType::DONE, kNull, 0));
    }
    else if (act == 3)
    {
        // Act 4 style: extract max from a Max Heap, sift the new root down.
        m_start = FakeSnapshots::heapFull();
        const int after[] = { 80, 70, 75, 40, 50, 60, 30, 10, 20, 90 };
        m_final = FakeSnapshots::makeHeap(after, 10, 9);
        m_targetId = 0;
        m_optimal = 8;

        Event ex = makeHeapEv(EventType::EXTRACT_MAX, 0, kNull, 0);
        ex.key = 90;
        ex.newV = 10;
        addEvent(ex);

        Event s1 = makeHeapEv(EventType::HEAP_SWAP, 0, 9, 1);
        s1.oldV = 90;
        s1.newV = 30;
        s1.result = SwapReason::Extract;
        addEvent(s1);

        Event ms = makeHeapEv(EventType::MARK_SORTED, 9, kNull, 0);
        ms.key = 90;
        addEvent(ms);

        Event sz = makeHeapEv(EventType::HEAP_SIZE_CHANGE, kNull, kNull, 0);
        sz.oldV = 10;
        sz.newV = 9;
        addEvent(sz);

        Event hs = makeHeapEv(EventType::HEAPIFY_START, 0, kNull, 0);
        hs.newV = 9;
        addEvent(hs);

        Event s2 = makeHeapEv(EventType::HEAP_SWAP, 0, 2, 1);
        s2.oldV = 30;
        s2.newV = 80;
        s2.result = SwapReason::SiftDown;
        addEvent(s2);

        Event s3 = makeHeapEv(EventType::HEAP_SWAP, 2, 6, 1);
        s3.oldV = 30;
        s3.newV = 75;
        s3.result = SwapReason::SiftDown;
        addEvent(s3);

        Event hd = makeHeapEv(EventType::HEAPIFY_DONE, 0, kNull, 0);
        hd.newV = 9;
        addEvent(hd);
        addEvent(makeEv(EventType::DONE, kNull, 0));
    }
    else
    {
        // Acts 1 and 2: BST search for 60 (path 50 -> 70 -> 60), prune the rest.
        m_start = FakeSnapshots::bst7();
        m_final = m_start;
        m_targetId = 5;
        m_optimal = 6;

        addEvent(makeEv(EventType::VISIT, 0, 1));
        Event c0 = makeEv(EventType::COMPARE, 0, 1);
        c0.key = 60;
        c0.result = CompareResult::Greater;
        addEvent(c0);

        addEvent(makeEv(EventType::VISIT, 4, 1));
        Event c1 = makeEv(EventType::COMPARE, 4, 1);
        c1.key = 60;
        c1.result = CompareResult::Less;
        addEvent(c1);

        addEvent(makeEv(EventType::VISIT, 5, 1));
        Event c2 = makeEv(EventType::COMPARE, 5, 1);
        c2.key = 60;
        c2.result = CompareResult::Equal;
        addEvent(c2);

        Event k1 = makeEv(EventType::HIGHLIGHT_SUBTREE, 1, 0);
        k1.result = HighlightMode::Skipped;
        addEvent(k1);
        Event k2 = makeEv(EventType::HIGHLIGHT_SUBTREE, 6, 0);
        k2.result = HighlightMode::Skipped;
        addEvent(k2);
        addEvent(makeEv(EventType::DONE, kNull, 0));
    }

    // capacity = ceil(1.5 x optimal) + 2  (INTEGRATION: computed by the model)
    m_alarmCap = (3 * m_optimal + 1) / 2 + 2;
}

void LevelScreen::startBatch()
{
    m_renderer.snapTo(m_start);
    m_renderer.clearHighlights();
    m_renderer.setTarget(kNull);
    m_alarm = 0;
    m_qIndex = 0;
    m_evActive = false;
    m_stepping = false;
    m_stepReq = false;
    m_paused = false;
    m_done = false;
    m_busy = (m_qCount > 0);
    m_log.addLine("--- play ---");
    refreshHud();
}

void LevelScreen::beginCurrent()
{
    Event& ev = m_queue[m_qIndex];
    m_renderer.beginEvent(ev);
    if (ev.type == EventType::ROTATE)
    {
        // structure change: nodes tween to the post-rotation slots
        m_renderer.animateTo(m_final);
    }
    m_log.addLine(describeEvent(ev));
    m_alarm += ev.alarmCost;
    m_evT = 0.f;
    m_evActive = true;
    refreshHud();
}

void LevelScreen::finishBatch()
{
    m_busy = false;
    m_evActive = false;
    m_done = true;
    m_renderer.animateTo(m_final);
    m_log.addLine("Batch complete.");
    refreshHud();
}

void LevelScreen::advanceDriver(float dt)
{
    if (!m_busy)
    {
        return;
    }

    if (!m_evActive)
    {
        if (m_paused && !m_stepReq)
        {
            return;
        }
        if (m_stepReq)
        {
            m_stepping = true;
            m_stepReq = false;
        }
        beginCurrent();
        return;
    }

    if (m_paused && !m_stepping)
    {
        return;
    }

    Event& ev = m_queue[m_qIndex];
    m_evT += dt * m_speed;
    float t = m_evT / evDuration(ev.type);
    if (t < 1.f)
    {
        m_renderer.progress(ev, t);
        return;
    }

    m_renderer.progress(ev, 1.f);
    m_renderer.endEvent(ev);
    m_evActive = false;
    m_stepping = false;
    m_qIndex++;
    if (m_qIndex >= m_qCount)
    {
        finishBatch();
    }
}

// ----------------------------------------------------------------- actions

void LevelScreen::onToolbarClick(void* context, int action)
{
    static_cast<LevelScreen*>(context)->onToolbar(action);
}

void LevelScreen::onToolbar(int action)
{
    if (action == ToolbarAction::Undo)
    {
        doUndo();
    }
    else if (action == ToolbarAction::Replay)
    {
        doReplay();
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

void LevelScreen::doUndo()
{
    // FAKE: undo = back to the state before the batch.
    // INTEGRATION: GameSession::apply(Undo) -> snapshot -> renderer.snapTo().
    m_busy = false;
    m_evActive = false;
    m_done = false;
    m_alarm = 0;
    m_renderer.snapTo(m_start);
    m_renderer.clearHighlights();
    m_log.addLine("Undo: restored the previous state.");
    refreshHud();
}

void LevelScreen::doReplay()
{
    startBatch();
}

void LevelScreen::doReset()
{
    m_busy = false;
    m_evActive = false;
    m_done = false;
    m_alarm = 0;
    m_renderer.snapTo(m_start);
    m_renderer.clearHighlights();
    m_renderer.setTarget(kNull);
    m_renderer.setSelected(kNull);
    m_renderer.fit(true);
    m_log.clear();
    m_log.addLine("Level reset.");
    refreshHud();
}

void LevelScreen::doHint()
{
    // INTEGRATION: hint comes from the model; also caps stars at 2 (HudState).
    m_renderer.setTarget(m_targetId);
    m_log.addLine("Hint used: the gold ring marks the room to look at.");
}

void LevelScreen::togglePlay()
{
    if (!m_busy)
    {
        startBatch();
        return;
    }
    m_paused = !m_paused;
    refreshHud();
}

void LevelScreen::stepOnce()
{
    if (!m_busy)
    {
        startBatch();
    }
    m_paused = true;
    m_stepReq = true;
    refreshHud();
}

void LevelScreen::refreshHud()
{
    // INTEGRATION: these numbers come from ActionResult.hud, never from GUI maths.
    int score = 1000 - 10 * m_alarm;
    if (score < 0)
    {
        score = 0;
    }
    m_hud.setAlarm(m_alarm, m_alarmCap);
    m_hud.setScore(score);
    m_hud.setSteps(m_alarm, m_alarmCap);

    if (m_busy && m_paused)
    {
        m_hud.setStatus("PAUSED");
    }
    else if (m_busy)
    {
        m_hud.setStatus("PLAYING");
    }
    else if (m_done)
    {
        m_hud.setStatus("DONE");
    }
    else
    {
        m_hud.setStatus("READY");
    }
}

// ------------------------------------------------------------------- input

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
        if (m_renderer.isHeap())
        {
            m_tooltip.showText(sx + 16.f, sy + 16.f,
                "value: " + std::to_string(key) + "\nindex: " + std::to_string(id)
                + "\ndepth: " + std::to_string(depth));
        }
        else
        {
            m_tooltip.showNode(sx + 16.f, sy + 16.f, key, height, bal, size, depth,
                m_renderer.kind() == TreeKind::AVL);
        }
        return;
    }

    int cell = m_renderer.pickCell(sx, sy);
    if (cell != kNull && m_renderer.nodeInfo(cell, key, height, size, bal, depth))
    {
        m_tooltip.showText(sx + 16.f, sy + 16.f,
            "array cell\nindex: " + std::to_string(cell) + "\nvalue: " + std::to_string(key));
        return;
    }
    m_tooltip.hide();
}

void LevelScreen::onCanvasClick(int mx, int my)
{
    if (m_busy)
    {
        return; // input locked while the animation plays
    }
    float sx = (float)mx;
    float sy = (float)my;
    int id = m_renderer.pick(sx, sy);
    if (id == kNull)
    {
        id = m_renderer.pickCell(sx, sy);
    }
    m_renderer.setSelected(id);
    if (id != kNull)
    {
        // INTEGRATION: build PlayerAction{ClickNode, id} and call GameSession::apply().
        m_log.addLine("Clicked #" + std::to_string(id));
    }
}

void LevelScreen::handleEvent(const sf::Event& e)
{
    // Esc is handled by UIManager.
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Space)
        {
            togglePlay();
            return;
        }
        if (e.key.code == sf::Keyboard::Right)
        {
            stepOnce();
            return;
        }
        if (e.key.code == sf::Keyboard::Left)
        {
            m_log.addLine("Step back needs undo data (comes with GameSession).");
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
    m_speed = m_toolbar.speed();
    m_toolbar.update(dt);
    m_hud.update(dt);
    m_log.update(dt);
    m_renderer.update(dt);
    advanceDriver(dt);
}

void LevelScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);
    window.draw(m_canvasBox);
    m_renderer.draw(window);
    window.draw(m_canvasHint);

    m_hud.draw(window);
    m_log.draw(window);
    m_toolbar.draw(window);
    m_tooltip.draw(window);
}
