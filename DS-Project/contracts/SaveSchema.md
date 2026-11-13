# Save file schema

Saved to `saves/progress.json` (gitignored). Uses the same JSON parser as levels.

```json
{
  "version": 1,
  "stars": [3, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  "endless": {
    "seed": 4242,
    "kind": "avl",
    "tree": "50 30 # # 70 60 # # #",
    "heap": [],
    "activeSize": 0,
    "bestScore": 1250
  }
}
```

## Rules

- `stars`: 20 ints, index = (act - 1) * 5 + (level - 1). Level is unlocked when the previous one has >= 1 star.
- Trees are stored as preorder with `#` for null: `50 30 # # 70 60 # # #`.
- Heaps are stored as the underlying array plus `activeSize` (`tree` is then empty).
- AVL `height` and `size` are NOT stored. They are recomputed on load.
- The Validator runs on the loaded structure before the game starts. If it fails, the endless state is discarded and campaign progress is kept.
- Leaderboard: `saves/leaderboard.json`, `{ "scores": [ { "name": "AAA", "score": 1200 } ] }`, top 10.
