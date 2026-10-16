# llvm-passes

Two LLVM passes written as out-of-tree plugins for LLVM 18's new pass manager, with lit and FileCheck tests.

| Pass | Type | What it does |
|------|------|--------------|
| `inst-counter` | Analysis | Prints the number of instructions in each function |
| `my-dce` | Transformation | Removes instructions whose results are unused and that have no side effects |

## Requirements

- Linux (tested on Ubuntu 24.04 under WSL)
- LLVM 18 (`clang-18`, `llvm-18-dev`, `llvm-18-tools`)
- CMake 3.20+ and Ninja
- lit (`pipx install lit`)

## Build

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++-18 \
  -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm
cmake --build build
```

This produces two plugins: `build/libInstCounter.so` and `build/libMyDCE.so`.

## Usage

Count instructions in each function:

```bash
opt-18 -load-pass-plugin=./build/libInstCounter.so \
  -passes=inst-counter -disable-output input.ll
```

Remove dead code and print the resulting IR:

```bash
opt-18 -load-pass-plugin=./build/libMyDCE.so -passes=my-dce -S input.ll
```

Measure the effect of dead code elimination by counting before and after:

```bash
opt-18 -load-pass-plugin=./build/libInstCounter.so \
  -load-pass-plugin=./build/libMyDCE.so \
  -passes=inst-counter,my-dce,inst-counter -disable-output input.ll
```

## Tests

```bash
lit -v test/
```

| Test | What it checks |
|------|----------------|
| `inst-counter.ll` | Counts a single-block function; skips declarations |
| `max.ll` | Counts instructions across multiple basic blocks |
| `dce.ll` | Removes a dead instruction and a chain of dead instructions; keeps a `store` and a call to an unknown function |

Tests use hand-written IR rather than Clang output, so they keep passing when Clang's code generation changes between versions.

## Design decisions

**Collect, then delete.** Erasing an instruction while iterating over its basic block invalidates the iterator and leads to undefined behavior. `my-dce` first collects dead instructions in a `SmallVector`, then erases them after iteration finishes. (LLVM's `make_early_inc_range` is an alternative that allows safe deletion inside the loop.)

**Iterate to a fixed point.** Deleting an instruction can make the instructions it used dead too, as in a chain like `%x = mul ...; %y = add %x, 1` where only `%y` is initially unused. The pass repeats collect-and-delete until a round removes nothing. Each round removes at least one instruction or ends the loop, so it always terminates.

**What is never deleted.** An instruction is removed only if its result is unused, it is not a terminator (such as `ret` or `br`), and it has no side effects. This keeps stores, calls to functions that may have side effects, and other instructions whose removal would change program behavior.

**Reporting changes correctly.** `inst-counter` only reads the IR, so it returns `PreservedAnalyses::all()`. `my-dce` returns `PreservedAnalyses::none()` only when it actually removed something, so LLVM knows when cached analyses must be recomputed.

## Limitations and future work

- Each round rescans the whole function. A worklist approach, which rechecks only the operands of erased instructions, would avoid repeated full scans on large functions.
- The pass does not remove unreachable basic blocks or dead stores (stores whose values are never read).