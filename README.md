# Data-Structures-Game

## Run the integrated project test runner

In VS Code, press **F5** and choose **Run Full Project Tests (Release x64)**. The pre-launch build compiles the SFML GUI, game/model modules, and the test suite into one executable, then starts `tests/test_main.cpp`'s doctest runner. The former SFML window entry point in `DS-Project/gui/main_gui.cpp` is disabled for this build.

To build and run only the model suite from a terminal, use:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .vscode/test-models.ps1
```
