# Vault Heist: The Living Vault

Data Structures project. 
A GUI puzzle-strategy game where the vault is a tree. Tree shape (height, balance, ordering) decides whether the thief escapes before the alarm fills. Campaign Acts 1 to 4 cover general binary trees, BST, AVL, and Max Heap / Heap Sort, plus Endless mode and a Sandbox.

## Requirements

- C++17 compiler: MSVC (Visual Studio 2022 or later) or g++ 11 or later.
- SFML **2.6.1** (the 2.x API). SFML 3 is not supported.
- No CMake. Build with the VS Code tasks (Windows) or the commands below.
- Tested on: [fill in OS, compiler version, SFML path].

## Project layout

| Folder | Contents |
|---|---|
| `contracts/` | Shared headers: events, containers, constants, view snapshot. No SFML. |
| `model/` | Pure data structures: BinaryTree, BST, AVLTree, MaxHeap, Sorts, Validator, Serializer. No GUI. |
| `game/` | Level loading and generation, GameSession (the bridge), scoring, save and leaderboard. No SFML. |
| `gui/` | SFML screens, widgets, renderer, animation. The only folder that includes SFML. |
| `tests/` | doctest unit tests (model and game logic). |
| `benchmarks/` | Benchmark harness (no GUI, events off by default). |
| `console/` | Optional console demo of the model. |
| `levels/` | `act1.json` to `act4.json` (campaign), `endless.json`, `GUIDE.md`. |
| `assets/` | Font and images. |
| `third_party/doctest/` | doctest single header. |
| `docs/` | Complexity analysis, benchmark report, test report, GUI screenshots, level guide, final report. |

The game loads `levels/` and `assets/` relative to the working directory, so **run the GUI from the `DS-Project` folder** (the folder that contains `assets/` and `levels/`).

## Build and run

### Windows, VS Code (recommended)

- GUI: Run and Debug panel, **Run Vault Heist GUI (Release x64)**, then the green start button.
- Model tests: **Run Model Tests (Release x64)**.
- Benchmarks: task **Run Model Benchmarks**.
- Console demo: task **Build and Run Console Model Demo**.

Command-line equivalent for the tests:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .vscode/test-models.ps1
```

### g++ (Linux, macOS, or Windows MinGW)

Run all commands from the `DS-Project` folder.

**GUI (Linux and macOS):**

```bash
g++ -std=c++17 -O2 -I gui gui/*.cpp gui/widgets/*.cpp gui/screens/*.cpp game/*.cpp model/*.cpp \
    -o vh_gui -lsfml-graphics -lsfml-window -lsfml-system
./vh_gui
```

**GUI (Windows MinGW):** copy the SFML `.dll` files next to the executable, then:

```bash
g++ -std=c++17 -O2 -I gui -I C:/SFML-2.6.1/include gui/*.cpp gui/widgets/*.cpp gui/screens/*.cpp \
    game/*.cpp model/*.cpp -o vh_gui.exe -L C:/SFML-2.6.1/lib \
    -lsfml-graphics -lsfml-window -lsfml-system
vh_gui.exe
```

**Model tests (doctest):** the test sources use paths such as `DS-Project/levels/...`, so run the binary from the folder **above** `DS-Project`:

```bash
cd DS-Project
g++ -std=c++17 -O2 -o model_tests tests/*.cpp model/*.cpp game/*.cpp
cd ..
./DS-Project/model_tests
```

**Benchmarks:**

```bash
cd DS-Project
g++ -std=c++17 -O2 -o bench benchmarks/*.cpp model/*.cpp
./bench --no-events
```

Use `./bench --events` only to check that event recording works. Timed results use `--no-events`.

**Console demo:**

```bash
cd DS-Project
g++ -std=c++17 -o console_demo console/console_main.cpp model/*.cpp
./console_demo
```

## Controls

### Global
- **Esc**: back one screen. From Briefing, Level, Heap Level or Result, go to the Campaign Map. Otherwise go to the Main Menu.

### Main Menu and Campaign Map
- Mouse: click buttons and banners. Click a banner once to select it, click it again (or Start) to open the briefing.
- **Left / Right** or **Q / E**: change act. **Enter**: start the selected level.

### Briefing
- **Enter** or **Space**: start the level. **Back** returns to the map.

### Level screen (Acts 1 to 3)
- **Space**: pause or resume the animation. **Right arrow**: step one event (while paused).
- **Z**: undo. **H**: hint (caps the result at two stars).
- Mouse wheel: zoom. Drag the background: pan. Hover a room: info tooltip.
- Input depends on the mission:
  - BST missions (2.1 to 2.3, 2.5): type a key in KEY, or click a room to fill it. Use Search, Insert, or the delete-case buttons. Delete asks you to name the case first.
  - 1.1 Traversal: click rooms in traversal order. The route submits automatically at the last room. Z removes the last pick.
  - 1.2 Reconstruct: drag a token from the shelf onto a placed room, then onto its left or right side. The root goes on an empty area.
  - 1.3 Height and diameter: type both numbers, then press Submit.
  - 1.4 LCA: click the meeting room.
  - 1.5 Mirror: press Mirror.
  - 2.4 Range raid: click every room whose key lies in the range shown in the prompt.
  - 3.1 Badges: click every unbalanced room.
  - 3.2 and 3.3 Repair: drag the pivot onto its left or right child, or click the pivot and press a rotation button.
  - 3.4 Auto: predict the rotation before each insert (None, LL, RR, LR, RL).
  - 3.5 Order statistic: click the path from the root down to the target.

### Heap Fortress (Act 4)
- Click an array cell or tree node to select it. Click an adjacent node (parent or child) to swap the two.
- Buttons: Build heap, Heapify selected, Extract max, Clear selection.
- Invalid swaps and invalid extractions cost alarm and a penalty.
- One-click Heap Sort is intentionally not available. Build and extract manually.

### Sandbox
- Pick a tree type with the tabs: Binary, BST, AVL, MaxHeap.
- Click a node or cell to fill field A. **Shift + click** fills field B.
- Use the operation buttons. Z undoes. Random and Clear rebuild the structure.

### Endless
- Type a key in KEY, then press Insert, Search or Delete. Correct moves add score and time. Mistakes subtract time.
- Scores go to `saves/leaderboard.json`.

### Settings
- Antialiasing and animation speed (applied immediately).

## Saves and runtime files

- `saves/progress.json`: campaign stars and endless state (created at runtime).
- `saves/leaderboard.json`: top ten local scores (created at runtime).
- Both are generated files. Do not include them in the submission ZIP.

## Known limitations

- Hints are limited, and some hints cap the result at two stars.
- Heap Sort is not a single action in Act 4 by design.
- Timings in `docs/P1_benchmark_report.md` come from one machine and are not portable guarantees.