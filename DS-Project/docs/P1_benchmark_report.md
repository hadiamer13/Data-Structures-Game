# Project 1 benchmark report

## Environment and method

Built with Visual Studio 2026 C++ tools, MSVC x64, C++17, Release optimization (`/O2`), on the project workspace machine. Timings use `std::clock()` and report CPU milliseconds. Event recording was disabled for timed runs; comparison, swap, and rotation counts were gathered independently where available. Inputs are deterministic and generated from fixed seeds. These timings are local observations, not cross-machine performance guarantees.

Reproduce with VS Code task **Run Model Benchmarks** or `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .vscode/bench.ps1`.

## Results

### Search and height (10,000 searches per row)

| n | Input | BST height | AVL height | BST average comparisons | AVL average comparisons | BST search ms | AVL search ms |
|---:|---|---:|---:|---:|---:|---:|---:|
| 100 | Sorted | 100 | 7 | 50.62 | 5.80 | 2 | <1 |
| 1,000 | Sorted | 1,000 | 10 | 503.51 | 8.98 | 20 | <1 |
| 10,000 | Sorted | 10,000 | 14 | 4,949.90 | 12.36 | 228 | 1 |
| 10,000 | Random | 33 | 16 | 17.22 | 12.58 | 1 | 1 |

Sorted input drives the BST height and search work to linear growth. AVL height remains logarithmic and limits search comparisons.

### AVL updates and order statistics

At `n=10,000`, sorted insertion averaged `0.9986` rotations per inserted key; deletion averaged `0.4987` rotations per deleted key, with at most one rotation in a single deletion for this sequence. Random insertion/deletion averaged `0.4596` / `0.1548` rotations, with a maximum of three rotations per deletion. Across 1,000 order-statistic queries at `n=10,000`, BST in-order selection took 24 ms and AVL `kth` took 6 ms.

### Heap construction and sorting

Bottom-up construction used 9,999 comparisons and no swaps for reverse input at `n=10,000`; the random input used 18,799 comparisons and 7,439 swaps. Heap sort on the random input used 226,535 comparisons and 116,782 swaps. Heap sort processed all four tested patterns: random, sorted, reverse, and duplicate-heavy.

### Sorting algorithms (n=10,000)

| Input | Algorithm | Comparisons | Time ms |
|---|---|---:|---:|
| Random | Insertion | 25,251,081 | 21 |
| Random | Merge | 120,516 | 1 |
| Random | Quick | 193,820 | <1 |
| Random | Heap sort | 235,372 | 2 |
| Reverse | Insertion | 49,995,000 | 41 |
| Reverse | Merge | 69,008 | <1 |
| Reverse | Quick | 143,616 | <1 |
| Reverse | Heap sort | 226,682 | 1 |

The measurement run confirmed the expected quadratic insertion-sort growth on random/reverse inputs and `O(n log n)` comparison counts for merge, quick, and heap sort on these cases. The harness also reports sorted and duplicate-heavy inputs, swaps, and auxiliary-space bounds.

## Limits

`std::clock()` has coarse resolution for fast operations, so many small rows display `0.000` or `1.000` ms and should not be over-interpreted. Operation counts and tree heights provide the more stable comparison. Timings include normal process variation and the report should be regenerated when comparing another machine or compiler.
