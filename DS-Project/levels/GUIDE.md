# Adding campaign levels

Every campaign level is data. Adding or changing a level needs **no code change**.
Optimal steps and the alarm capacity are never written in the files: the model computes
them (`ceil(1.5 x optimal) + 2`).

## Files

| File | Purpose |
|---|---|
| `levels/act1.json` ... `act4.json` | The campaign. One file per act, exactly five levels each. |
| `levels/endless.json` | Endless Survival settings (see the last section). |

## Steps to add or edit a level

1. Open the act file (`act2.json`, for example).
2. Copy an existing entry inside `levels` and edit its fields. Keep `index` between 1 and 5,
   in order (1, 2, 3, 4, 5). The loader rejects a file that does not have exactly five levels.
3. Pick a `mission` from the table below and set `param` as shown.
4. Run the game from the project root (so `levels/` resolves). A malformed file is reported
   on the Campaign Map with the character position of the error.
5. Run the unit tests. The suite loads all four files and checks every level.

## Fields

| Field | Type | Meaning |
|---|---|---|
| `index` | int | 1..5, position inside the act |
| `name`, `concept`, `objective` | string | Shown on the Campaign Map and Briefing |
| `treeKind` | string | `binary`, `bst`, `avl`, `maxheap` |
| `mission` | string | See the mission table |
| `seed` | int >= 0 | `srand(seed)` before generation: the level is reproducible and gradable |
| `nodeCount` | int | Tree or array size |
| `style` | string | `random`, `skewed`, `sorted`, `reverse`, `heap`, `duplicates` |
| `param` | int | Mission specific (table below) |
| `boss` | bool | Shows the boss tag |
| `preorder` | string | Optional fixed tree, e.g. `"50 30 # # 70 # #"`. Empty = generate from the seed |

The JSON parser accepts objects, arrays, strings, signed 32-bit integers, booleans and null.
It rejects floats, duplicate keys, Unicode escapes, comments and trailing data.

## Missions and `param`

| Mission | Act | `param` | `nodeCount` |
|---|---|---|---|
| `traversal` | 1 | 0 preorder, 1 inorder, 2 postorder, 3 level order | 6..15 |
| `reconstruct` | 1 | 0 | 6..15, values are distinct |
| `heightDiameter`, `lcaPath`, `mirror` | 1 | 0 | 6..15 |
| `bstSearch` | 2 | target key (0 = deepest key) | any |
| `bstInsert` | 2 | key to insert (0 = max + 1) | any |
| `bstDelete` | 2 | key to delete (0 = deepest key) | any |
| `bstRange`, `bstBoss` | 2 | `bstBoss`: target key. Use `style: "skewed"` for the corridor | any |
| `avlBadges` | 3 | 0 | 5..15 |
| `avlSingle` | 3 | 0 = LL or RR chosen from the seed, or force with 0 (LL) / 1 (RR) | any |
| `avlDouble` | 3 | 2 (LR) or 3 (RL) | any |
| `avlAuto` | 3 | number of inserts to predict (2..12) | 3..12 start keys |
| `avlOrderStat` | 3 | k, the k-th most valuable treasure | 3..63 |
| `heapBuild`, `heapRepair`, `heapExtract` | 4 | 0 | 3..31 |
| `heapRace` | 4 | operation limit | 3..31 |
| `heapBoss` | 4 | 0 | 3..31 |

Act 3 keeps the rule that each rotation type (LL, RR, LR, RL) appears at least twice across
the act. Act 4 input styles: `random`, `reverse`, `heap` (already a heap), `duplicates`.

## Example entry

```json
{ "index": 2, "name": "Stash the Loot", "concept": "BST insertion",
  "objective": "Insert the target key.", "treeKind": "bst", "mission": "bstInsert",
  "seed": 202, "nodeCount": 9, "style": "random", "param": 17,
  "boss": false, "preorder": "" }
```

## Endless mode (`endless.json`)

| Field | Meaning |
|---|---|
| `treeKind` | Structure to survive on (`avl` by default) |
| `seed`, `startSize`, `keyRange` | Reproducible starting tree and the key range used by the stream |
| `insertPercent` | Share of inserts in the mixed insert/delete stream (the rest are deletes) |
| `phases` | Difficulty ramp. Once `afterSeconds` have passed, one operation arrives every `secondsPerOperation` seconds, `burst` at a time. Phases must be in increasing `afterSeconds` order |
| `scorePerSecond` | Survival score |
| `failOnInvalidTree` | The run ends when the Validator reports the tree invalid |

Saved endless runs (tree or heap, seed, best score) live in `saves/progress.json`
as described in `contracts/SaveSchema.md`. The local top ten is `saves/leaderboard.json`.