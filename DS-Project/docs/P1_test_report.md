# Project 1 test report

The model test suite was built and run with the repository's `.vscode/test-models.ps1` script using Visual Studio 2026 MSVC x64 and C++17.

**Result:** 19 test cases passed; 1,195 assertions passed; 0 failed; 0 skipped.

Command: `g++ -std=c++17 -O2 -o model_tests.exe tests/*.cpp model/*.cpp game/*.cpp` then `.\model_tests.exe` from the `DS-Project` folder.
Compiler: g++ 14.2.0, run on 20-11-2026.

The suite includes existing tree, heap, serializer, validator, scoring, and game-session tests plus new coverage for custom insertion/merge/quick sorts, JSON parsing and error positions, all four five-level act files, seeded tree generation, AVL rotation demonstrations, save/load round trips, and top-ten local leaderboard persistence.

The Release x64 SFML game and the model test runner build as separate executables. The game compiles the GUI, game, and model sources; the test runner compiles `tests/test_main.cpp` and the model/game tests. The suite passed all 14 cases and 1,031 assertions. The optional model console demo is built and run with the **Build and Run Console Model Demo** VS Code task.
