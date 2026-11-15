# Adding campaign levels

Each act has one JSON file, `act1.json` through `act4.json`, with an `act` number and exactly five entries in `levels`. Copy an entry and keep its `index` between 1 and 5. The loader checks field types, recognized names, ordering, and the five-level count.

Every level entry has:

- `name`, `concept`, and `objective`: player-facing text.
- `treeKind`: `binary`, `bst`, `avl`, or `maxheap`.
- `mission`: a name listed in `contracts/LevelSchema.md`.
- `seed`: non-negative integer used by the generator to reproduce the level.
- `nodeCount`: generated tree or heap size.
- `style`: `random`, `skewed`, `sorted`, `reverse`, `heap`, or `duplicates`.
- `param`: mission-specific integer, such as a traversal index, target key, kth value, or operation limit.
- `boss`: whether the campaign map marks the level as a boss.
- `preorder`: optional fixed tree snapshot; use an empty string for generated data.

The parser accepts objects, arrays, strings, signed 32-bit integers, booleans, and null. It rejects floating-point values, duplicate object keys, Unicode escapes, and trailing data, and reports the character position of syntax errors.
