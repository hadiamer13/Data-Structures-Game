# Project 1 test report

The model test suite was built and run with the repository's `.vscode/test-models.ps1` script using Visual Studio 2026 MSVC x64 and C++17.

**Result:** 14 test cases passed; 1,031 assertions passed; 0 failed; 0 skipped.

The suite includes existing tree, heap, serializer, validator, scoring, and game-session tests plus new coverage for custom insertion/merge/quick sorts, JSON parsing and error positions, all four five-level act files, seeded tree generation, AVL rotation demonstrations, save/load round trips, and top-ten local leaderboard persistence.

The integrated Release x64 build compiled the SFML GUI, game/model modules, and all test sources into one executable with `tests/test_main.cpp` as its entry point. The same executable ran successfully with all 14 test cases passing. The optional model console demo is built and run with the **Build and Run Console Model Demo** VS Code task.
