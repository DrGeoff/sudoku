# TODO

## Current Objective

(none — CLI fix + C++26 modernization shipped; see git log)

## In Progress

- (none)

## Next

- (awaiting new work)

## Blocked

- (none)

## Context

- Build: `source ~/compiletools/.venv/bin/activate && ct-cake` (variant gcc.cxx26.debug; g++ is clang 21.1.8, -std=c++26)
- C++ LSP: clangd 21.1.8 on PATH, pi-lens `cpp` server auto-selected, CDB = ct-cake's `compile_commands.json`. Probe: `lens_diagnostics source=lsp` (cold start ~30s).
- Regression baselines kept in `.cache/old-solutions/` (94 solution CSVs from the pre-change binary) — delete if no longer needed.
- Exit codes: 0 success, 1 runtime error, 2 usage error.
- Infra note: long token sequences (paths, guard macros) got mangled in tool inputs this session — after any write, grep-verify include guards and long identifiers. Subagent runs are flaky here (503/timeout hit twice); keep delegated tasks bounded.
