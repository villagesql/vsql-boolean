# VillageSQL Boolean Extension

A real `BOOLEAN` type for VillageSQL. Replaces `BOOL`/`BOOLEAN` aliases for
`TINYINT(1)` with a proper boolean type — clear metadata, standard truth
values, and correct dump/restore semantics.

## Building

**Linux:**
```bash
mkdir -p build
cmake -S . -B build -DVillageSQL_BUILD_DIR=$HOME/build/villagesql
cmake --build build
cmake --install build
```

**macOS:**
```bash
mkdir -p build
cmake -S . -B build -DVillageSQL_BUILD_DIR=~/.villagesql/build
cmake --build build
cmake --install build
```

Or use the convenience script:

```bash
VillageSQL_BUILD_DIR=/path/to/villagesql/build bash build.sh
```

## Installing

```sql
INSTALL EXTENSION vsql_boolean;
```

## Quick Start

```sql
CREATE TABLE settings (id INT, enabled STRICTBOOL);
INSERT INTO settings VALUES (1, 'true'), (2, 'false'), (3, 'yes'), (4, '0');
SELECT * FROM settings WHERE enabled = 'true';
SHOW CREATE TABLE settings;
```

## Function Reference

### Aggregate functions

| Function | Returns | Description |
|---|---|---|
| `boolean_sum(col)` | `INT` | Count of `TRUE` values in the group. NULLs are skipped. |
| `boolean_avg(col)` | `REAL` | Ratio of `TRUE` values (`true_count / total`). Returns `NULL` on an empty group. NULLs are skipped. |

```sql
SELECT boolean_sum(enabled), boolean_avg(enabled) FROM settings;
-- boolean_sum(enabled): 2
-- boolean_avg(enabled): 0.5
```

### STRICTBOOL type

Storage: 1 byte on disk (`0x00` = FALSE, `0x01` = TRUE).

**Accepted input** (case-insensitive):

| Input | Stored as |
|---|---|
| `'true'`, `'t'`, `'yes'`, `'on'`, `'1'` | TRUE |
| `'false'`, `'f'`, `'no'`, `'off'`, `'0'` | FALSE |
| `NULL` | NULL |

**Output**: `'true'` or `'false'` (lowercase).

**Ordering**: `FALSE` sorts before `TRUE`; NULLs first with `ASC`.

**NULL handling**: NULL inputs are preserved as NULL. For `NOT NULL` columns
receiving NULL with IGNORE, the intrinsic default is `'false'`.

## Known Limitations

**Type is named `STRICTBOOL`, not `BOOLEAN`.** MySQL's parser translates
`BOOLEAN` and `BOOL` to `TINYINT(1)` at the grammar level before VEF type
resolution runs. A column declared as `BOOLEAN` creates a `tinyint(1)` column
even with this extension installed. The VEF type is therefore named
`STRICTBOOL`. When VillageSQL adds support for VEF types to override built-in
type aliases, a rename will be possible without changing the binary storage
format. Track this at https://github.com/villagesql/villagesql-server/issues/604.

**Built-in `SUM()` and `AVG()` are not supported on `STRICTBOOL` columns.**
These aggregates require numeric promotion that the current VEF API does not
expose for custom types. Use `boolean_sum()` and `boolean_avg()` instead (see
above). `COUNT(*)`, `MIN()`, and `MAX()` work correctly. Track native aggregate
support at https://github.com/villagesql/villagesql-server/issues/605.

**Extension-defined index types are not supported in the stable VEF API.**
Custom STRICTBOOL columns participate in standard MySQL B-tree indexes via the
compare function. Bitmap indexes or other index types optimized for boolean
columns require the index type registration API. Track at
https://github.com/villagesql/villagesql-server/issues/264.

**Uninstalling requires no dependent columns.** `UNINSTALL EXTENSION
vsql_boolean` fails if any table has a `STRICTBOOL` column. Drop or alter
those columns first, then uninstall, then reinstall. There is no
`ALTER EXTENSION` command. Track upgrade support at
https://github.com/villagesql/villagesql-server/issues/12.

## Testing

See `TESTING.md`.

## Contributing

See the [VillageSQL Contributing Guide](https://github.com/villagesql/villagesql-server/blob/main/CONTRIBUTING.md).

## Reporting Bugs and Requesting Features

Open an issue at https://github.com/villagesql/villagesql-server/issues

## Contact

- Discord: https://discord.gg/KSr6whd3Fr
- GitHub Issues: https://github.com/villagesql/villagesql-server/issues

## License

GPL-2.0. See source files for the full license header.
