# BACKLOG

## Near-term

- `data/smh/hard/smh.1521.csv` is truncated (8 rows / 72 cells) — needs the missing 9th row from the source (smh.1521) or deletion. Owner: user.

## Later

- `Cell::initial()` getter, `Grid::get(row,col)`, `writeRawChain`, `PrintCellLocationInformation` are unused public interface — candidates for deletion once confirmed dead.
- `usesGrid` static member on rule structs is declared but never read.
- Consider `std::expected` for `Grid::parse` instead of exceptions (house style currently exceptions).

## Ideas

- `--version` flag (not requested; skipped).
- `--quiet` flag to suppress the per-rule explanation output (not requested; skipped).
