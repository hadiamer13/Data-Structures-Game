# Level JSON schema

Supported JSON subset (our own parser): objects, arrays, strings, integers, booleans, null.
No floats, no unicode escapes, no comments. Parse errors report the character position.

One file per act: `levels/act1.json` ... `levels/act4.json`, plus `levels/endless.json`.

```json
{
  "act": 1,
  "levels": [
    {
      "index": 1,
      "name": "Guard Patrol",
      "concept": "Preorder, inorder, postorder, level order",
      "objective": "Click the rooms in the order the patrol visits them.",
      "treeKind": "binary",
      "mission": "traversal",
      "seed": 101,
      "nodeCount": 8,
      "style": "random",
      "param": 0,
      "boss": false,
      "preorder": ""
    }
  ]
}
```

## Fields

| Field | Type | Meaning |
|---|---|---|
| index | int | 1..5 position inside the act |
| name, concept, objective | string | Shown on Campaign Map and Briefing |
| treeKind | string | `binary`, `bst`, `avl`, `maxheap` (maps to `TreeKind::*`) |
| mission | string | See table below (maps to `MissionKind::*`) |
| seed | int | `srand(seed)` at generation, so the level is reproducible |
| nodeCount | int | Tree or array size (Act 1: 6..15) |
| style | string | `random`, `skewed`, `sorted`, `reverse`, `heap`, `duplicates` |
| param | int | Mission specific (traversal: 0 pre, 1 in, 2 post, 3 level; kth: k; heap race: op limit) |
| boss | bool | Shows the boss tag |
| preorder | string | Optional fixed tree, e.g. `"50 30 # # 70 # #"`. Empty = generate from seed |

## Mission names

traversal, reconstruct, heightDiameter, lcaPath, mirror, bstSearch, bstInsert, bstDelete,
bstRange, bstBoss, avlBadges, avlSingle, avlDouble, avlAuto, avlOrderStat, heapBuild,
heapRepair, heapExtract, heapRace, heapBoss, endless.

## Rules

- Optimal steps are NEVER written in the file. The model computes them.
- Adding a level = add an object to the act's `levels` array. No code change.
