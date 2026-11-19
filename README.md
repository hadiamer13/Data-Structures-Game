# Data-Structures-Game

## Run the game

In VS Code, choose **Run Vault Heist GUI (Release x64)** in the Run and Debug panel, then click the green start button. This builds the SFML GUI and starts it from `DS-Project` so its assets load. If F5 controls your laptop volume, use the on-screen start button or **Run → Start Debugging**.

## Run the model tests

Choose **Run Model Tests (Release x64)** in the Run and Debug panel, or run the test task from the Terminal menu. The test runner is a separate executable built from `tests/test_main.cpp`.

To build and run the tests from a terminal, use:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .vscode/test-models.ps1
```
