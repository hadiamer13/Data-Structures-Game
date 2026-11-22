#include "TestSupport.h"
#include "../game/GameSession.h"

static PlayerAction action(int kind, int value)
{
    PlayerAction result;
    result.kind = kind;
    result.a = value;
    return result;
}

void runScoreTests()
{
    ScoreSystem score;
    score.configure(4, "objective");
    testCheck(score.alarmCap() == 8, "alarm capacity is ceil(1.5 times optimal) plus two");
    score.recordAction(2);
    score.recordAction(1, 20);
    score.markWon();
    testCheck(score.score() == 950 && score.stars() == 2, "score subtracts alarm and penalty points and awards two stars");

    score.configure(2, "objective");
    score.recordAction(1);
    score.recordAction(1);
    score.markWon();
    testCheck(score.stars() == 3 && score.score() == 980, "optimal penalty-free completion earns three stars");

    score.configure(4, "objective");
    score.recordAction(0);
    score.recordAction(0);
    score.recordAction(0);
    score.recordAction(0);
    score.recordAction(0);
    score.recordAction(0);
    score.markWon();
    testCheck(score.stars() == 1, "completion beyond the two-star step limit earns one star");

    score.configure(2, "objective");
    score.recordAction(2);
    score.recordAction(2);
    score.recordAction(1);
    testCheck(score.status() == HudStatus::Lost && score.alarm() == score.alarmCap() && score.stars() == 0,
              "reaching alarm capacity loses the level");
}

void runGameSessionTests()
{
    LevelDef level;
    level.mission = MissionKind::BstSearch;
    level.param = 12;
    level.objective = "Find key 12";
    level.preorder = "8 4 # # 12 # #";
    GameSession session;
    testCheck(session.startLevel(level) && session.tree().nodeCount() == 3, "GameSession loads a valid starting tree");
    ActionResult search = session.apply(action(ActionKind::Search, 12));
    testCheck(search.accepted && search.hud.status == HudStatus::Won && search.hud.alarm == 2,
              "search action returns events and completes its target mission");

    LevelDef sandbox;
    sandbox.objective = "Practice";
    sandbox.treeKind = TreeKind::BST;
    sandbox.mission = MissionKind::Endless;   // no mission rules, so every BST action is allowed
    sandbox.preorder = "8 4 # # 12 # #";
    testCheck(session.startLevel(sandbox), "GameSession can restart a level");
    testCheck(session.apply(action(ActionKind::Insert, 6)).accepted && session.canUndo(), "insert records an undo snapshot");
    testCheck(session.tree().nodeCount() == 4, "insert updates the model");
    testCheck(session.apply(action(ActionKind::Undo, 0)).accepted && session.tree().nodeCount() == 3,
              "undo restores a serialized tree snapshot");
    testCheck(session.apply(action(ActionKind::Delete, 4)).accepted && session.tree().nodeCount() == 2,
              "delete updates the model");
    testCheck(session.apply(action(ActionKind::Reset, 0)).accepted && session.tree().nodeCount() == 3 && session.hud().alarm == 0,
              "reset restores the initial state and clears score progress");
    ViewSnapshot view = session.snapshot();
    testCheck(view.kind == TreeKind::BST && view.rootId == 0 && view.nodes[0].key == 8,
              "GameSession exposes a GUI snapshot without transferring ownership");
}
