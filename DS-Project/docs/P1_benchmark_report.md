# Project 1 benchmark report

## Environment and method

Compiler: [output of `g++ --version`], C++17, `-O2`, on [OS and CPU].
Timings use `std::clock()` (CPU milliseconds) and have roughly 1 ms resolution, so values near 0 are not meaningful. Event recording was disabled (`--no-events`). Comparison, swap and rotation counts are operation counts recorded by the model. Inputs are deterministic and generated from fixed seeds.

Reproduce with:

```powershell
g++ -std=c++17 -O2 -o bench.exe benchmarks/*.cpp model/*.cpp
.\bench.exe --no-events > docs\P1_benchmark_output.txt
```

The full output is in `docs/P1_benchmark_output.txt`. Every number in this report comes from that file.

## Results

### Height and search (10,000 searches per row)

| n | Input | BST height | AVL height | BST avg comparisons | AVL avg comparisons | BST search ms | AVL search ms |
|---:|---|---:|---:|---:|---:|---:|---:|
| 100 | Sorted | 100 | 7 | 50.62 | 5.80 | 7 | 1 |
| 100 | Random | 17 | 8 | 9.71 | 5.87 | 2 | 0 |
| 1,000 | Sorted | 1,000 | 10 | 503.51 | 8.98 | 88 | 2 |
| 1,000 | Random | 28 | 12 | 13.04 | 9.23 | 4 | 3 |
| 10,000 | Sorted | 10,000 | 14 | 4,949.90 | 12.36 | 1,060 | 6 |
| 10,000 | Random | 33 | 16 | 17.22 | 12.58 | 13 | 12 |

Sorted insertion turns the BST into a chain, so its height and search cost grow linearly with n. AVL height stays logarithmic.

### AVL rotations

| n | Input | Avg rotations per insert | Avg rotations per delete | Max rotations in one delete |
|---:|---|---:|---:|---:|
| 1,000 | Sorted | 0.9900 | 0.4910 | 1 |
| 1,000 | Random | 0.4630 | 0.1540 | 3 |
| 10,000 | Sorted | 0.9986 | 0.4987 | 1 |
| 10,000 | Random | 0.4596 | 0.1548 | 3 |

### Order statistics (1,000 queries)

| n | BST inorder selection (ms) | AVL `kth` (ms) |
|---:|---:|---:|
| 1,000 | 3 | 0 |
| 10,000 | 89 | 6 |

### Heap construction and heap sort (n = 10,000)

Source: the `[Max Heap build and Heap Sort]` section. Each heap sort figure is the total for one build plus one full sort.

| Input | Build comparisons | Build swaps | Heap sort comparisons | Heap sort swaps |
|---|---:|---:|---:|---:|
| Random | 18,799 | 7,439 | 226,535 | 116,782 |
| Sorted | 19,982 | 9,992 | 234,477 | 121,964 |
| Reverse | 9,999 | 0 | 226,682 | 116,696 |
| Duplicates | 17,700 | 6,508 | 200,842 | 100,599 |

Reverse input is already a max heap, so construction needs no swaps. Sorted (ascending) input is the worst case for construction.

### Sorting algorithms (n = 10,000)

Source: the `[Heap Sort vs insertion, merge and quick sort]` section.

| Input | Algorithm | Comparisons | Swaps | Time (ms) |
|---|---|---:|---:|---:|
| Random | Insertion | 25,251,081 | n/a | 58 |
| Random | Merge | 120,516 | n/a | 3 |
| Random | Quick | 193,820 | 30,497 | 1 |
| Random | Heap sort | 235,372 | 124,226 | 7 |
| Sorted | Insertion | 9,999 | n/a | 0 |
| Sorted | Merge | 64,608 | n/a | 1 |
| Sorted | Quick | 143,615 | 0 | 0 |
| Sorted | Heap sort | 244,460 | 131,956 | 8 |
| Reverse | Insertion | 49,995,000 | n/a | 213 |
| Reverse | Merge | 69,008 | n/a | 1 |
| Reverse | Quick | 143,616 | 5,000 | 1 |
| Reverse | Heap sort | 226,682 | 116,696 | 6 |
| Duplicates | Insertion | 21,494,906 | n/a | 78 |
| Duplicates | Merge | 114,242 | n/a | 1 |
| Duplicates | Quick | 158,904 | 9,015 | 1 |
| Duplicates | Heap sort | 207,943 | 106,872 | 7 |

Insertion sort and merge sort count element moves rather than swaps, so their swap column is n/a. The harness's insertion swap counter is never incremented.

**Heap sort consistency note.** The heap sort rows here use the sorting harness, which generates random and duplicates arrays with a different seed from the heap section. Reverse matches exactly. The sorted row does not match and remains unexplained; see the note in the working notes.

## Analysis

Sorted BST height grows linearly (100 to 10,000 keys gives heights 100 to 10,000), and its search cost follows. AVL height stays between 7 and 16 across all tested sizes. On random input the BST stays shallow, so its search cost is close to the AVL cost.

Insertion sort's comparisons grow quadratically on random and reverse input (25.3 million and 50.0 million at n = 10,000). Merge, quick and heap sort stay near n log n. Heap sort uses O(1) auxiliary space, while merge sort uses O(n).

## Limits

`std::clock()` resolution is about 1 ms, so the timing columns are indicative only. Comparison and swap counts are the reliable comparison. Timings vary between runs, which is visible in this report: for example, BST search at n = 10,000 sorted was 228 ms in an earlier run and 1,060 ms in this one. Regenerate the report when the machine or compiler changes.