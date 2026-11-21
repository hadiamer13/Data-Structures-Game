// gui/screens/SandboxScreen.cpp
#include "SandboxScreen.h"
#include "../UIManager.h"
#include "../Theme.h"
#include "../EventText.h"

namespace
{
    const float TabH = 44.f;
    const float ToolbarH = 48.f;
    const float LogW = 320.f;
    const float PanelW = 250.f;
    const float ClickSlop = 5.f;

    namespace ScreenOp
    {
        inline const int Random = 100;
        inline const int Clear = 101;
    }

    struct OpDef
    {
        int op;
        const char* label;
    };

    const OpDef kBinaryOps[] =
        {
            {SandboxOp::Insert, "Insert"}, {SandboxOp::Search, "Search"}, {SandboxOp::Preorder, "Preorder"}, {SandboxOp::Inorder, "Inorder"}, {SandboxOp::Postorder, "Postorder"}, {SandboxOp::LevelOrder, "Level"}, {SandboxOp::Mirror, "Mirror"}, {SandboxOp::Info, "Info"}, {ScreenOp::Random, "Random"}, {ScreenOp::Clear, "Clear"}};
    const OpDef kBstOps[] =
        {
            {SandboxOp::Insert, "Insert"}, {SandboxOp::Search, "Search"}, {SandboxOp::Delete, "Delete"}, {SandboxOp::Range, "Range"}, {SandboxOp::Min, "Min"}, {SandboxOp::Max, "Max"}, {SandboxOp::Succ, "Succ"}, {SandboxOp::Pred, "Pred"}, {SandboxOp::Kth, "Kth"}, {SandboxOp::Info, "Info"}, {ScreenOp::Random, "Random"}, {ScreenOp::Clear, "Clear"}};
    const OpDef kAvlOps[] =
        {
            {SandboxOp::Insert, "Insert"}, {SandboxOp::Search, "Search"}, {SandboxOp::Delete, "Delete"}, {SandboxOp::Range, "Range"}, {SandboxOp::RotLL, "Rot LL"}, {SandboxOp::RotRR, "Rot RR"}, {SandboxOp::RotLR, "Rot LR"}, {SandboxOp::RotRL, "Rot RL"}, {SandboxOp::Kth, "Kth"}, {SandboxOp::Rank, "Rank"}, {SandboxOp::CountRange, "Count"}, {SandboxOp::Info, "Info"}, {ScreenOp::Random, "Random"}, {ScreenOp::Clear, "Clear"}};
    const OpDef kHeapOps[] =
        {
            {SandboxOp::Insert, "Insert"}, {SandboxOp::Extract, "Extract"}, {SandboxOp::Build, "Build"}, {SandboxOp::Heapify, "Heapify"}, {SandboxOp::SiftUp, "SiftUp"}, {SandboxOp::Swap, "Swap"}, {SandboxOp::Sort, "Sort"}, {SandboxOp::Info, "Info"}, {ScreenOp::Random, "Random"}, {ScreenOp::Clear, "Clear"}};

    const char* const TabNames[4] = {"Binary", "BST", "AVL", "MaxHeap"};

    bool needsA(int op)
    {
        return op == SandboxOp::Insert || op == SandboxOp::Search || op == SandboxOp::Delete ||
               op == SandboxOp::Range || op == SandboxOp::Kth || op == SandboxOp::Rank ||
               op == SandboxOp::CountRange || op == SandboxOp::Succ || op == SandboxOp::Pred ||
               op == SandboxOp::Heapify || op == SandboxOp::SiftUp || op == SandboxOp::Swap;
    }

    bool needsB(int op)
    {
        return op == SandboxOp::Range || op == SandboxOp::CountRange || op == SandboxOp::Swap;
    }

    bool isRotation(int op)
    {
        return op >= SandboxOp::RotLL && op <= SandboxOp::RotRL;
    }

    bool animated(int type)
    {
        return type != EventType::UPDATE_HEIGHT && type != EventType::UPDATE_SIZE;
    }

    bool structural(int type)
    {
        return type == EventType::INSERT || type == EventType::UNLINK || type == EventType::ROTATE;
    }

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

    void styleText(sf::Text& t, const sf::Font& font, unsigned size, const sf::Color& col, float x, float y)
    {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setPosition(x, y);
    }
}

// constructor

SandboxScreen::SandboxScreen(AppContext& ctx)
    : m_ctx(ctx), m_renderer(ctx.assets->font()), m_anim(&m_renderer)
{
    for (int i = 0; i < kTabs; ++i)
    {
        m_tabCtx[i].owner = this;
        m_tabCtx[i].kind = i;
    }
    for (int i = 0; i < kMaxOps; ++i)
    {
        m_opCtx[i].owner = this;
        m_opCtx[i].op = 0;
    }

    buildLayout();
    m_renderer.setRingTexture(&ctx.assets->nodeRing());
    m_anim.setCallbacks(&SandboxScreen::onAnimEvent, &SandboxScreen::onAnimDone, this);
    loadKind(TreeKind::BST, false);
}

// button idher udher karne ke ham 25000 dollar lete hai

void SandboxScreen::buildLayout()
{
    const sf::Font& font = m_ctx.assets->font();
    const float W = (float)Theme::WindowW;
    const float H = (float)Theme::WindowH;

    const sf::Texture& bg = m_ctx.assets->campaignBackground();
    m_background.setTexture(bg);
    m_background.setScale(W / (float)bg.getSize().x, H / (float)bg.getSize().y);
    m_dim.setSize(sf::Vector2f(W, H));
    m_dim.setFillColor(sf::Color(0, 0, 0, 190));

    m_tabBar.setPosition(0.f, 0.f);
    m_tabBar.setSize(sf::Vector2f(W, TabH));
    m_tabBar.setFillColor(Theme::TooltipBg);
    m_tabBar.setOutlineColor(Theme::BtnOutline);
    m_tabBar.setOutlineThickness(-2.f);

    styleText(m_title, font, 24, Theme::TitleColor, 14.f, 6.f);
    m_title.setString("SANDBOX");

    for (int i = 0; i < kTabs; ++i)
    {
        float cx = LogW + 90.f + (float)i * 170.f;
        m_tab[i].init(TabNames[i], font, sf::Vector2f(cx, TabH * 0.5f), sf::Vector2f(150.f, 32.f));
        m_tab[i].setCallback(&SandboxScreen::onTabClick, &m_tabCtx[i]);
    }
    m_tabUnder.setSize(sf::Vector2f(150.f, 3.f));
    m_tabUnder.setFillColor(Theme::NodeGlow);

    float cx = LogW;
    float cy = TabH;
    float cw = W - LogW - PanelW;
    float ch = H - TabH - ToolbarH;
    m_canvasBox.setPosition(cx, cy);
    m_canvasBox.setSize(sf::Vector2f(cw, ch));
    m_canvasBox.setFillColor(sf::Color(12, 6, 9, 170));
    m_canvasBox.setOutlineColor(Theme::BtnOutline);
    m_canvasBox.setOutlineThickness(-2.f);
    m_renderer.setCanvasRect(cx, cy, cw, ch);

    styleText(m_hint, font, 12, sf::Color(170, 150, 150), cx + 10.f, cy + ch - 20.f);
    m_hint.setString("Wheel: zoom  Drag: pan  Click: select (fills A)  Shift+click: fills B  Space: pause  Z: undo");
    styleText(m_msg, font, 15, Theme::StarOn, cx + 10.f, cy + 8.f);

    float px = W - PanelW;
    m_panel.setPosition(px, TabH);
    m_panel.setSize(sf::Vector2f(PanelW, ch));
    m_panel.setFillColor(Theme::TooltipBg);
    m_panel.setOutlineColor(Theme::BtnOutline);
    m_panel.setOutlineThickness(-2.f);

    m_fieldA.init("A: key / lo / k / index", font, px + 15.f, TabH + 34.f, PanelW - 30.f, 30.f);
    m_fieldB.init("B: hi / 2nd index", font, px + 15.f, TabH + 94.f, PanelW - 30.f, 30.f);
    m_fieldA.setCallback(&SandboxScreen::onFieldClick, this);
    m_fieldB.setCallback(&SandboxScreen::onFieldClick, this);

    styleText(m_validText, font, 13, Theme::TextColor, px + 15.f, H - ToolbarH - 190.f);
    styleText(m_stepsText, font, 14, Theme::TextColor, px + 15.f, H - ToolbarH - 56.f);
    styleText(m_selText, font, 14, Theme::TextColor, px + 15.f, H - ToolbarH - 32.f);

    m_log.init(font, 0.f, TabH, LogW, ch);
    m_toolbar.init(font, 0.f, H - ToolbarH, W, ToolbarH);
    m_toolbar.setCallback(&SandboxScreen::onToolbarClick, this);
    m_tooltip.init(font);
}

// ------------------------------------------------------------ kind / ops

void SandboxScreen::loadKind(int kind, bool empty)
{
    m_kind = kind;
    m_selected = kNull;
    m_pendingFocus = kNull;
    m_anim.clear();
    m_log.clear();
    m_fieldA.clear();
    m_fieldB.clear();

    m_session.start(kind, m_seed, empty);
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setSelected(kNull);
    m_renderer.setTarget(kNull);
    m_renderer.fit(false);

    rebuildOps();
    refreshTabs();
    refreshInfo();
    m_log.addLine(std::string("Sandbox: ") + TabNames[kind] + (empty ? " (empty)" : (" (seed " + std::to_string(m_seed) + ")")));
    setMessage(empty ? "Empty structure. Type a value in A and press Insert."
                     : "Ready. Type a value in A, then press an operation.");
}

void SandboxScreen::rebuildOps()
{
    const OpDef* defs = kBstOps;
    int n = (int)(sizeof(kBstOps) / sizeof(kBstOps[0]));
    if (m_kind == TreeKind::Binary)
    {
        defs = kBinaryOps;
        n = (int)(sizeof(kBinaryOps) / sizeof(kBinaryOps[0]));
    }
    else if (m_kind == TreeKind::AVL)
    {
        defs = kAvlOps;
        n = (int)(sizeof(kAvlOps) / sizeof(kAvlOps[0]));
    }
    else if (m_kind == TreeKind::MaxHeap)
    {
        defs = kHeapOps;
        n = (int)(sizeof(kHeapOps) / sizeof(kHeapOps[0]));
    }
    if (n > kMaxOps)
    {
        n = kMaxOps;
    }

    const sf::Font& font = m_ctx.assets->font();
    const float px = (float)Theme::WindowW - PanelW;
    const float y0 = TabH + 160.f;
    const float rowH = 38.f;
    const float margin = 15.f; // inner margin of the panel
    const float gap = 10.f;    // space between the two columns
    const float btnH = 30.f;

    m_opCount = n;

    // Pass 1: create the buttons with a small label and padding, and find each column's widest button.
    float colW[2] = {0.f, 0.f};
    for (int i = 0; i < n; ++i)
    {
        const int col = i % 2;
        m_opCtx[i].op = defs[i].op;
        m_op[i].init(defs[i].label, font, sf::Vector2f(0.f, 0.f), sf::Vector2f(100.f, btnH));
        m_op[i].setTextSize(16);
        m_op[i].setPadding(8.f);
        m_op[i].setCallback(&SandboxScreen::onOpClick, &m_opCtx[i]);
        if (m_op[i].width() > colW[col])
        {
            colW[col] = m_op[i].width();
        }
    }

    // Pass 2: place each column's buttons centred in that column.
    const float colX[2] =
        {
            px + margin + colW[0] * 0.5f,
            px + margin + colW[0] + gap + colW[1] * 0.5f};
    for (int i = 0; i < n; ++i)
    {
        const int col = i % 2;
        const int row = i / 2;
        m_op[i].setCenter(sf::Vector2f(colX[col], y0 + (float)row * rowH));
    }
}

void SandboxScreen::refreshTabs()
{
    for (int i = 0; i < kTabs; ++i)
    {
        std::string label = TabNames[i];
        if (i == m_kind)
        {
            label = "[ " + label + " ]"; // not colour alone: brackets + underline
        }
        m_tab[i].setLabel(label.c_str());
    }
    m_tabUnder.setPosition(LogW + 90.f + (float)m_kind * 170.f - 75.f, TabH - 5.f);
}

void SandboxScreen::refreshInfo()
{
    m_stepsText.setString("Visits + compares: " + std::to_string(m_session.steps()));
    if (m_selected == kNull)
    {
        m_selText.setString("Selected: none");
    }
    else
    {
        m_selText.setString((m_renderer.isHeap() ? "Selected: index " : "Selected: room #") + std::to_string(m_selected));
    }
    std::string text;
    bool ok = m_session.check(text);
    setValid(text, ok);
}

void SandboxScreen::setValid(const std::string& text, bool ok)
{
    m_validText.setString(wrapChars(text, 30));
    m_validText.setFillColor(ok ? sf::Color(120, 220, 140) : sf::Color(255, 110, 100));
}

void SandboxScreen::setMessage(const std::string& msg)
{
    m_msg.setString(wrapChars(msg, 68));
    if (!msg.empty())
    {
        m_log.addLine("> " + msg);
    }
}

bool SandboxScreen::typing() const
{
    return m_fieldA.focused() || m_fieldB.focused();
}

// ---------------------------------------------------------------- callbacks

void SandboxScreen::onTabClick(void* context)
{
    TabCtx* c = static_cast<TabCtx*>(context);
    c->owner->onTab(c->kind);
}

void SandboxScreen::onOpClick(void* context)
{
    OpCtx* c = static_cast<OpCtx*>(context);
    c->owner->onOp(c->op);
}

void SandboxScreen::onToolbarClick(void* context, int action)
{
    static_cast<SandboxScreen*>(context)->onToolbar(action);
}

void SandboxScreen::onFieldClick(void* context)
{
    static_cast<SandboxScreen*>(context)->onFieldEnter();
}

void SandboxScreen::onAnimEvent(void* context, const Event& e, int index, int total)
{
    (void)index;
    (void)total;
    static_cast<SandboxScreen*>(context)->m_log.addLine(describeEventText(e));
}

void SandboxScreen::onAnimDone(void* context)
{
    SandboxScreen* s = static_cast<SandboxScreen*>(context);
    s->m_renderer.setTarget(s->m_pendingFocus);
}

void SandboxScreen::onTab(int kind)
{
    if (kind != m_kind)
    {
        loadKind(kind, false);
    }
}

void SandboxScreen::onOp(int op)
{
    doOp(op);
}

void SandboxScreen::onFieldEnter()
{
    doOp(SandboxOp::Insert); // Enter = insert
}

void SandboxScreen::onToolbar(int action)
{
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
        setMessage("Hints are not available in the sandbox.");
    }
}

// ------------------------------------------------------------------ actions

void SandboxScreen::doOp(int op)
{
    if (m_anim.busy())
    {
        m_anim.skipAll(); // finish the running animation first
    }

    if (op == ScreenOp::Random)
    {
        m_seed++;
        loadKind(m_kind, false);
        return;
    }
    if (op == ScreenOp::Clear)
    {
        loadKind(m_kind, true);
        return;
    }

    int a = 0;
    int b = 0;
    if (needsA(op))
    {
        if (!m_fieldA.hasValue())
        {
            setMessage("Type a value in A first.");
            return;
        }
        a = m_fieldA.value();
    }
    if (needsB(op))
    {
        if (!m_fieldB.hasValue())
        {
            setMessage("This operation needs a value in B as well.");
            return;
        }
        b = m_fieldB.value();
    }
    if ((op == SandboxOp::Range || op == SandboxOp::CountRange) && a > b)
    {
        int t = a;
        a = b;
        b = t;
    }
    if (isRotation(op))
    {
        if (m_selected == kNull)
        {
            setMessage("Click a room first: it is the pivot.");
            return;
        }
        a = m_selected;
    }

    m_renderer.setTarget(kNull);
    SandboxResult r = m_session.apply(op, a, b);
    setMessage(r.message);
    setValid(r.validText, r.valid);
    m_stepsText.setString("Visits + compares: " + std::to_string(m_session.steps()));
    play(r);
}

// Plays one batch. The session only returns the state before and after, so the LAST structural
// event (INSERT / UNLINK / ROTATE) carries the "after" snapshot; label-only events are dropped.
void SandboxScreen::play(const SandboxResult& r)
{
    m_pendingFocus = r.focusId;

    int kept = 0;
    int lastStruct = -1;
    for (int i = 0; i < r.events.size(); ++i)
    {
        int t = r.events[i].type;
        if (!animated(t))
        {
            continue;
        }
        kept++;
        if (structural(t))
        {
            lastStruct = i;
        }
    }

    if (kept == 0)
    {
        m_anim.clear();
        m_renderer.animateTo(r.after);
        m_renderer.setTarget(m_pendingFocus);
        return;
    }

    m_renderer.snapTo(r.before);
    m_anim.beginBatch(r.before);
    for (int i = 0; i < r.events.size(); ++i)
    {
        if (!animated(r.events[i].type))
        {
            continue;
        }
        m_anim.enqueue(r.events[i], (i == lastStruct) ? &r.after : nullptr);
    }
    m_log.addLine("--- play ---");
    m_anim.endBatch(r.after);
}

void SandboxScreen::doUndo()
{
    m_anim.clear();
    bool ok = m_session.undo();
    m_renderer.snapTo(m_session.snapshot()); // hard sync, no stale positions
    m_renderer.setTarget(kNull);
    m_selected = kNull;
    m_renderer.setSelected(kNull);
    refreshInfo();
    setMessage(ok ? "Undone." : "Nothing to undo.");
}

void SandboxScreen::doReset()
{
    m_anim.clear();
    m_session.reset();
    m_renderer.snapTo(m_session.snapshot());
    m_renderer.setTarget(kNull);
    m_renderer.setSelected(kNull);
    m_renderer.fit(true);
    m_selected = kNull;
    m_log.clear();
    refreshInfo();
    setMessage("Reset to the starting state.");
}

// -------------------------------------------------------------------- input

void SandboxScreen::updateTooltip(int mx, int my)
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
                               "value: " + std::to_string(key) + "\nindex: " + std::to_string(id) + "\ndepth: " + std::to_string(depth));
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

void SandboxScreen::onCanvasClick(int mx, int my)
{
    float sx = (float)mx;
    float sy = (float)my;
    int id = m_renderer.pick(sx, sy);
    if (id == kNull)
    {
        id = m_renderer.pickCell(sx, sy);
    }
    m_selected = id;
    m_renderer.setSelected(id);

    int key = 0;
    int h = 0;
    int s = 0;
    int b = 0;
    int d = 0;
    if (id != kNull && m_renderer.nodeInfo(id, key, h, s, b, d))
    {
        bool second = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        int value = m_renderer.isHeap() ? id : key; // tree: key, heap: array index
        if (second)
        {
            m_fieldB.setValue(value);
        }
        else
        {
            m_fieldA.setValue(value);
        }
    }
    refreshInfo();
}

void SandboxScreen::handleEvent(const sf::Event& e)
{
    // Esc is handled by UIManager.
    m_fieldA.handleEvent(e);
    m_fieldB.handleEvent(e);

    if (e.type == sf::Event::KeyPressed && !typing())
    {
        if (e.key.code == sf::Keyboard::Space)
        {
            if (m_anim.busy())
            {
                m_anim.togglePause();
            }
            return;
        }
        if (e.key.code == sf::Keyboard::Right)
        {
            if (m_anim.busy())
            {
                m_anim.pause();
                m_anim.stepOne();
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
            setMessage("Hints are not available in the sandbox.");
            return;
        }
    }

    m_toolbar.handleEvent(e);
    for (int i = 0; i < kTabs; ++i)
    {
        m_tab[i].handleEvent(e);
    }
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].handleEvent(e);
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

void SandboxScreen::update(float dt)
{
    m_toolbar.update(dt);
    for (int i = 0; i < kTabs; ++i)
    {
        m_tab[i].update(dt);
    }
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].update(dt);
    }
    m_fieldA.update(dt);
    m_fieldB.update(dt);
    m_log.update(dt);
    m_renderer.update(dt);

    m_anim.setSpeed(m_toolbar.speed());
    m_anim.update(dt);
}

void SandboxScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_background);
    window.draw(m_dim);

    window.draw(m_canvasBox);
    m_renderer.draw(window);
    window.draw(m_hint);
    window.draw(m_msg);

    window.draw(m_tabBar);
    window.draw(m_title);
    for (int i = 0; i < kTabs; ++i)
    {
        m_tab[i].draw(window);
    }
    window.draw(m_tabUnder);

    window.draw(m_panel);
    m_fieldA.draw(window);
    m_fieldB.draw(window);
    for (int i = 0; i < m_opCount; ++i)
    {
        m_op[i].draw(window);
    }
    window.draw(m_validText);
    window.draw(m_stepsText);
    window.draw(m_selText);

    m_log.draw(window);
    m_toolbar.draw(window);
    m_tooltip.draw(window);
}
