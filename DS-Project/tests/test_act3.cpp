#include "TestSupport.h"
#include "../game/GameSession.h"

static LevelDef avlLevel(int mission, int param, const char* preorder)
{
    LevelDef d;
    d.mission = mission;
    d.param = param;
    d.objective = "test";
    d.preorder = preorder;
    return d;
}

static PlayerAction click(int id)
{
    PlayerAction a;
    a.kind = ActionKind::ClickNode;
    a.a = id;
    return a;
}

static PlayerAction rotate(int pivot, int type)
{
    PlayerAction a;
    a.kind = ActionKind::Rotate;
    a.a = pivot;
    a.b = type;
    return a;
}

void runAct3SessionTests()
{
    // 3.2 single rotation: wrong case first, then the right one.
    GameSession single;
    testCheck(single.startLevel(avlLevel(MissionKind::AvlSingle, RotationType::LL, "")), "3.2 level starts from a template");
    ActionResult wrong = single.apply(rotate(0, RotationType::RR));
    testCheck(!wrong.accepted && wrong.hud.penalties == 50 && wrong.hud.alarm == 2 &&
        wrong.hud.status == HudStatus::Playing, "wrong rotation costs 2 alarm and 50 points");
    ActionResult right = single.apply(rotate(0, RotationType::LL));
    testCheck(right.accepted && right.hud.status == HudStatus::Won && single.avlTree().isBalanced(),
        "LL repair balances the tree and wins");

    // 3.3 double rotation: both rotations run in one action.
    GameSession dbl;
    testCheck(dbl.startLevel(avlLevel(MissionKind::AvlDouble, RotationType::LR, "")), "3.3 level starts");
    ActionResult lr = dbl.apply(rotate(0, RotationType::LR));
    testCheck(lr.accepted && lr.hud.status == HudStatus::Won && dbl.avlTree().isBalanced(),
        "LR repair balances the tree");
    int rotates = 0;
    for (int i = 0; i < lr.events.size(); i++)
    {
        if (lr.events[i].type == EventType::ROTATE)
        {
            rotates++;
        }
    }
    testCheck(rotates == 2, "LR emits two ROTATE events");

    GameSession rl;
    rl.startLevel(avlLevel(MissionKind::AvlDouble, RotationType::RL, ""));
    testCheck(rl.apply(rotate(0, RotationType::RL)).hud.status == HudStatus::Won, "RL repair wins");
    GameSession rr;
    rr.startLevel(avlLevel(MissionKind::AvlSingle, RotationType::RR, ""));
    testCheck(rr.apply(rotate(0, RotationType::RR)).hud.status == HudStatus::Won, "RR repair wins");

    // 3.1 badges: only the root of this tree is violated.
    GameSession badges;
    testCheck(badges.startLevel(avlLevel(MissionKind::AvlBadges, 0, "50 30 20 10 # # # 40 # # 60 # #")),
        "3.1 level starts from a preorder");
    ActionResult miss = badges.apply(click(1));
    testCheck(!miss.accepted && miss.hud.status == HudStatus::Playing, "a balanced room is a wrong click");
    ActionResult hit = badges.apply(click(0));
    testCheck(hit.accepted && hit.hud.status == HudStatus::Won, "marking the last unbalanced room wins");

    // 3.4 prediction: always answering "no rotation" must still end the level.
    LevelDef autoDef;
    autoDef.mission = MissionKind::AvlAuto;
    autoDef.objective = "test";
    autoDef.seed = 5;
    GameSession autoSession;
    testCheck(autoSession.startLevel(autoDef) && autoSession.nextInsertKey() != kNull, "3.4 level starts with a queue");
    for (int i = 0; i < 6 && autoSession.hud().status == HudStatus::Playing; i++)
    {
        autoSession.apply(rotate(kNull, RotationType::None));
    }
    testCheck(autoSession.hud().status != HudStatus::Playing, "3.4 ends after the last insert");

    // 3.5 order statistic: 1st most valuable of keys 1..7 is 7 (path 4 -> 6 -> 7).
    GameSession order;
    LevelDef od = avlLevel(MissionKind::AvlOrderStat, 1, "4 2 1 # # 3 # # 6 5 # # 7 # #");
    testCheck(order.startLevel(od) && order.hud().optimalSteps == 3, "3.5 optimal steps = path length");
    testCheck(!order.apply(click(1)).accepted, "starting anywhere but the root is wrong");
    testCheck(order.apply(click(0)).accepted && order.apply(click(4)).accepted, "descends root then right");
    testCheck(order.apply(click(6)).hud.status == HudStatus::Won, "reaching the target wins");
}