# Project 1 complexity analysis

Let `n` be the number of stored keys and `h` the height of a tree. The bounds below describe the algorithms in the model layer; event recording adds work proportional to the events emitted and is disabled in timed benchmark runs.

## Trees

| Operation | Binary tree | BST | AVL tree |
|---|---:|---:|---:|
| Search | `O(n)` | Average `O(log n)`, worst `O(n)` | `O(log n)` |
| Insert / delete | `O(1)` when the target slot/node is known; locating a slot is `O(n)` | Average `O(log n)`, worst `O(n)` | `O(log n)` |
| Traversal / clear | `O(n)` | `O(n)` | `O(n)` |
| Height | `O(n)` to calculate | `O(n)` to calculate | `O(1)` from maintained root metadata |
| `k`th smallest | Not ordered | `O(n)` in this implementation | `O(log n)` using maintained subtree sizes |

BST costs depend on insertion order. Sorted insertion produces a chain, so operations that follow a root-to-leaf path take `O(n)`. AVL rotations keep height logarithmic after insertions and deletions; each update takes `O(log n)` and may perform a bounded number of local rotations. `AVLTree::kth` uses subtree-size metadata to descend one path.

## Heap and sorting

| Algorithm / operation | Time | Extra space |
|---|---:|---:|
| Bottom-up max-heap construction | `O(n)` | `O(1)` beyond the backing array |
| Heap insert / extract maximum | `O(log n)` | `O(1)` |
| Heap sort | `O(n log n)` | `O(1)` |
| Insertion sort | Best `O(n)`; average and worst `O(n^2)` | `O(1)` |
| Merge sort | `O(n log n)` | `O(n)` |
| Quicksort | Average `O(n log n)`; worst `O(n^2)` | `O(log n)` stack in this implementation |

Quicksort partitions around the middle element and recurses into the smaller partition, processing the larger partition iteratively. This keeps its call stack logarithmic even though adversarial partitions can still make its total comparison count quadratic.

## Measurement method

Run `Run Model Benchmarks` from VS Code or execute `.vscode/bench.ps1`. Inputs use a fixed local pseudo-random generator and fixed seeds. The report prints elapsed CPU time in milliseconds and operation/event counts separately. Use operation counts to compare algorithmic work; sub-millisecond timings are sensitive to scheduler and timer resolution. Event recording defaults to off for timing; `--events` enables it when invoking the built benchmark executable directly.
