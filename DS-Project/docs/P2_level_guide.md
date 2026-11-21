# How to add a campaign level

Every campaign level is data. Adding or editing a level needs **no code change** as long as its mission already exists. The model computes optimal steps and alarm capacity, so you never type them in.

## Files

| File | Purpose |
|---|---|
| `levels/act1.json` to `act4.json` | The campaign. One file per act, exactly five levels each. |
| `levels/endless.json` | Endless Survival settings. |
| `levels/GUIDE.md` | Full technical reference (this guide is the short version). |

## Steps

1. Open the act file, for example `levels/act2.json`.
2. Copy an existing entry inside `levels` and edit it. Keep `index` at 1 to 5, in order. The loader rejects a file that does not contain exactly five levels.
3. Choose a `mission` from the table below and set `param` as described.
4. Run the GUI from the `DS-Project` folder. A malformed file is reported on the Campaign Map with the character position of the error.
5. Run the model tests. They load all four act files and check the levels.

## Fields

| Field | Type | Meaning |
|---|---|---|
| `index` | int | 1 to 5, position inside the act. |
| `name`, `concept`, `objective` | string | Shown on the Campaign Map and Briefing. |
| `treeKind` | string | `binary`, `bst`, `avl`, `maxheap`. |
| `mission` | string | One of the names in the table below. |
| `seed` | int, 0 or more | Seed for generation. The same seed gives the same level. |
| `nodeCount` | int | Tree or array size. |
| `style` | string | `random`, `skewed`, `sorted`, `reverse`, `heap`, `duplicates`. |
| `param` | int | Mission-specific value (table below). |
| `boss` | bool | Shows the boss tag. |
| `preorder` | string | Optional fixed tree, e.g. `"50 30 # # 70 # #"`. Empty means generate from the seed. |

The JSON parser accepts objects, arrays, strings, signed 32-bit integers, booleans and null. It rejects floats, duplicate keys, Unicode escapes, comments and trailing data.

## Missions and `param`

| Mission | Act | `param` | Size |
|---|---|---|---|
| `traversal` | 1 | 0 preorder, 1 inorder, 2 postorder, 3 level order | 6 to 15 rooms |
| `reconstruct` | 1 | 0 = preorder + inorder, 1 = postorder + inorder | 6 to 15, distinct values |
| `heightDiameter`, `lcaPath`, `mirror` | 1 | 0 | 6 to 15 rooms |
| `bstSearch` | 2 | target key (0 = deepest key) | any |
| `bstInsert` | 2 | key to insert (0 = max + 1) | any |
| `bstDelete` | 2 | key to delete (0 = deepest key) | any |
| `bstRange` | 2 | **lo × 1000 + hi**, for example `4009` means lo = 4, hi = 9 (hi must be below 1000) | any |
| `bstBoss` | 2 | target key. Use `style: "skewed"` for the corridor | any |
| `avlBadges` | 3 | 0 | 5 to 15 |
| `avlSingle` | 3 | 0 = chosen from seed, 1 = LL, 2 = RR (check against the code if you change it) | any |
| `avlDouble` | 3 | 2 = LR, 3 = RL | any |
| `avlAuto` | 3 | number of inserts to predict (2 to 12) | 3 to 12 start keys |
| `avlOrderStat` | 3 | k, the k-th most valuable treasure | 3 to 63 |
| `heapBuild`, `heapRepair`, `heapExtract` | 4 | 0 | 3 to 31 |
| `heapRace` | 4 | operation limit | 3 to 31 |
| `heapBoss` | 4 | 0 | 3 to 31 |

Rules to keep:
- Act 3 must still contain each rotation type (LL, RR, LR, RL) at least twice across the act.
- Act 4 levels must use one of the input styles `random`, `reverse`, `heap` or `duplicates`.

## Example

```json
{ "index": 2, "name": "Stash the Loot", "concept": "BST insertion",
  "objective": "Insert the target key.", "treeKind": "bst", "mission": "bstInsert",
  "seed": 202, "nodeCount": 9, "style": "random", "param": 17,
  "boss": false, "preorder": "" }
```

## Adding a new mission type (needs code)

A new mission is not data-only. You must:
1. Add the name and value to `MissionKind` in `contracts/GameTypes.h`.
2. Add the name to the mission table in `game/LevelLoader.cpp`.
3. Add its behaviour in `game/GameSession.cpp` (setup and `apply` handling).
4. Add the optimal-step rule in `game/LevelManager.cpp` if it is not a BST, AVL or heap mission.
5. Add a playthrough test and update this table.

## Endless mode (`levels/endless.json`)

| Field | Meaning |
|---|---|
| `treeKind` | Structure to survive on (`avl` by default). |
| `seed`, `startSize`, `keyRange` | Reproducible starting tree and key range. |
| `insertPercent` | Share of inserts in the stream. The rest are deletes. |
| `phases` | Difficulty ramp. Each phase starts at `afterSeconds` and sends one operation every `secondsPerOperation` seconds, `burst` at a time. Phases must be in increasing order. |
| `scorePerSecond` | Survival score rate. |
| `failOnInvalidTree` | The run ends if the Validator reports an invalid tree. |