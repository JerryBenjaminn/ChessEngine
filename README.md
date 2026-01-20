# ChessEngine

## Square Indexing

Squares use 0..63 indexing where a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63.

## Build

```
cmake -S . -B build
cmake --build build
ctest --test-dir build
```