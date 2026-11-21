#include <cstdio>
#include <string>

#include "TestSupport.h"
#include "../game/Json.h"
#include "../game/Leaderboard.h"
#include "../game/LevelGenerator.h"
#include "../game/LevelLoader.h"
#include "../game/SaveManager.h"
#include "../model/Validator.h"

void runCampaignSupportTests()
{
    JsonDocument document;
    JsonError jsonError;
    testCheck(JsonParser::parse("{\"a\":[1,true,\"ok\\n\"]}", document, &jsonError), "JSON parser reads nested values and escapes");
    int parsedInteger = 0;
    bool parsedBoolean = false;
    std::string parsedString;
    const JsonValue* nested = document.root()->get("a");
    testCheck(nested != nullptr && nested->size() == 3 && nested->at(0)->asInteger(parsedInteger) && parsedInteger == 1 &&
                  nested->at(1)->asBoolean(parsedBoolean) && parsedBoolean && nested->at(2)->asString(parsedString) && parsedString == "ok\n",
              "JSON values retain their parsed types");
    testCheck(!JsonParser::parse("{\"a\":1,\"a\":2}", document, &jsonError) && jsonError.position > 0,
              "JSON parser reports duplicate keys with a position");

    LevelDefArray levels;
    std::string error;
    bool loadedAllActs = true;
    for (int act = 1; act <= 4; act++)
    {
        const std::string path = std::string("DS-Project/levels/act") + static_cast<char>('0' + act) + ".json";
        LevelDefArray actLevels;
        loadedAllActs = loadedAllActs && LevelLoader::loadFile(path, actLevels, &error) && actLevels.size() == 5;
        if (actLevels.size() > 0)
        {
            loadedAllActs = loadedAllActs && actLevels[0].act == act && actLevels[0].index == 1;
        }
    }
    testCheck(loadedAllActs, "all four shipped campaign files load five levels");

    LevelDef generated;
    generated.seed = 7123;
    generated.nodeCount = 15;
    generated.style = 0;
    BinaryTree binaryA;
    BinaryTree binaryB;
    const bool binaryGenerated = LevelGenerator::generateBinaryTree(generated, binaryA) &&
                                 LevelGenerator::generateBinaryTree(generated, binaryB);
    testCheck(binaryGenerated && Validator::validateBinaryTree(binaryA) && Validator::validateBinaryTree(binaryB) &&
                  binaryA.nodeCount() == 15 && binaryB.nodeCount() == 15,
              "seeded binary generation creates valid repeatable-size trees");

    LevelDef showcase;
    showcase.seed = 29;
    AVLTree avl;
    EventLog rotationLog;
    const bool showcaseGenerated = LevelGenerator::generateAVLRotationShowcase(showcase, avl, &rotationLog);
    int rotations[4] = {0, 0, 0, 0};
    for (int i = 0; i < rotationLog.size(); i++)
    {
        const Event& event = rotationLog.at(i);
        if (event.type == EventType::ROTATE && event.result >= RotationType::LL && event.result <= RotationType::RL)
        {
            rotations[event.result]++;
        }
    }
    testCheck(showcaseGenerated && avl.isBalanced() && rotations[RotationType::LL] >= 2 &&
                  rotations[RotationType::RR] >= 2 && rotations[RotationType::LR] >= 2 && rotations[RotationType::RL] >= 2,
              "AVL showcase emits each rotation pattern twice");

    SaveData saveData;
    saveData.stars[0] = 3;
    saveData.endlessSeed = 84;
    saveData.bestScore = 1200;
    const char* savePath = "DS-Project/tests/.phase78-save-test.json";
    SaveData roundTrip;
    const bool saveRoundTrip = SaveManager::save(savePath, saveData, &error) &&
                               SaveManager::load(savePath, roundTrip, &error) &&
                               roundTrip.stars[0] == 3 && roundTrip.bestScore == 1200;
    std::remove(savePath);
    testCheck(saveRoundTrip, "save manager round-trips campaign progress and endless metadata");

    Leaderboard leaderboard;
    bool added = true;
    for (int i = 0; i < 12; i++)
    {
        added = added && leaderboard.add(std::string("Player") + static_cast<char>('A' + i), i * 100);
    }
    const char* leaderboardPath = "DS-Project/tests/.phase78-leaderboard-test.json";
    Leaderboard loadedLeaderboard;
    const bool leaderboardRoundTrip = added && leaderboard.size() == Leaderboard::Capacity &&
                                      leaderboard.save(leaderboardPath, &error) &&
                                      loadedLeaderboard.load(leaderboardPath, &error) &&
                                      loadedLeaderboard.size() == Leaderboard::Capacity &&
                                      loadedLeaderboard.at(0).score == 1100;
    std::remove(leaderboardPath);
    testCheck(leaderboardRoundTrip, "local leaderboard retains the highest ten scores after reload");
}
