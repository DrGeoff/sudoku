# AGENTS.md

Guidance for coding agents working in this repository.

## What this is

A human-style Sudoku solver in C++26. Sources live in `src/`, with a test in
`src/test/`. The built binary is published at `bin/src/sudoku` (a copy of
`bin/<variant>/src/sudoku`, which is the symlink into the `cas-exedir/`
content-addressed cache).

## Building — use ct-cake

Compilation is done with **ct-cake** (from the `compiletools` project), not a
hand-written Makefile/CMake. There is no build config to edit: `ct-cake`
auto-detects source files, traces `#include` dependencies, builds the
executable, and runs tests.

### Get the venv on PATH first

`ct-cake` lives in a virtualenv that is **not** on the default PATH. Source it
before building:

```bash
source ~/compiletools/.venv/bin/activate
```

That puts `ct-cake` (and the other `ct-*` tools) on PATH. Verify with:

```bash
which ct-cake    # -> ~/compiletools/.venv/bin/ct-cake
ct-cake --version
```

If `~/compiletools` does not exist on this machine, the venv has not been set
up here — see the `compiletools` repo's `INSTALL` to create it. Do not try to
build with a system compiler as a workaround.

### Build

From the repo root:

```bash
ct-cake
```

- Quiet on a cache hit (prints only the banner); use `ct-cake -v` to see the
  compile/link steps and diagnostics.
- Default variant is `gcc.cxx26.debug`. Select others with
  `ct-cake --variant=gcc,release` (axes compose: toolchain, linker,
  optimization, sanitizer — e.g. `--variant=clang,debug,asan`).
- Object/linker artefacts are content-addressed in `cas-objdir/` and
  `cas-exedir/`; `compile_commands.json` is regenerated for IDE integration.
  Both are derived — do not edit or commit changes to them by hand.

### Run the result

```bash
./bin/src/sudoku puzzle.k
```

The argument is a **puzzle filename**, not a puzzle string. `Grid::parse`
(`src/grid.hpp`) dispatches on the last character: `v` → CSV, `k` → SDK;
anything else throws. It is not a `--help`-style CLI.

## Notes for agents

- Prefer `ct-cake -v` when a build fails so the actual compiler error is
  visible, not just the banner.
- `src/test/test_combinator.cpp` is the test entry point; `ct-cake` builds and
  runs it as part of the normal build.
- Keep changes to `src/` only; the build system needs no touching.
