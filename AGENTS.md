# AGENTS.md

This file provides guidance to AI coding assistants when working with code in this repository.

**Note**: Also check `AGENTS.local.md` for additional local development instructions when present.

## Project Overview

vsql-boolean adds a real `BOOLEAN` custom type to VillageSQL. It replaces the
`BOOL`/`BOOLEAN` aliases for `TINYINT(1)` with a proper boolean type that has
clear metadata, standard truth values, and correct dump/restore semantics.

Install name: `vsql_boolean`
SQL type: `STRICTBOOL`
Storage: 1 byte — `0x00` = FALSE, `0x01` = TRUE

## Build System

Set `VillageSQL_BUILD_DIR` to your VillageSQL build directory, then:

```bash
VillageSQL_BUILD_DIR=/path/to/villagesql/build bash build.sh
```

Or manually:

```bash
mkdir -p build
cmake -S . -B build -DVillageSQL_BUILD_DIR=/path/to/villagesql/build
cmake --build build
cmake --install build
```

The build produces `vsql_boolean.veb` and installs it to the VEB directory.

## Architecture

**Core Components:**
- `src/vsql_boolean.cc` — STRICTBOOL type: encode, decode, compare, hash + registration
- `manifest.json` — Extension metadata
- `CMakeLists.txt` — CMake build configuration
- `cmake/FindVillageSQL.cmake` — CMake module for finding VillageSQL SDK
- `mysql-test/t/` — MTR test files (`.test`)
- `mysql-test/r/` — MTR expected results (`.result`)

**Type Operations:**
- `STRICTBOOL::from_string` — parses string literals into 1-byte binary (case-insensitive: true/false/t/f/yes/no/on/off/1/0)
- `STRICTBOOL::to_string` — converts binary to "true" or "false"
- `STRICTBOOL::compare` — returns -1/0/1 (FALSE < TRUE)
- `STRICTBOOL::hash` — returns 0 for FALSE, 1 for TRUE

**Dependencies:**
- VillageSQL Extension Framework (VEF) — typed C++ API via `<villagesql/vsql.h>`
- C++ compiler with C++17 support

## Extension Installation

```sql
INSTALL EXTENSION 'vsql_boolean';
```

Then use it:

```sql
CREATE TABLE t (id INT, active STRICTBOOL);
INSERT INTO t VALUES (1, 'true'), (2, 'false'), (3, 'yes'), (4, '0');
SELECT * FROM t WHERE active = 'true';
SHOW CREATE TABLE t;
```

## VillageSQL Extension Framework (VEF) API Pattern

Extensions use the typed C++ API: include `<villagesql/vsql.h>`. Use typed
wrappers (`StringArg`, `CustomArg`, `CustomResult`, `StringResult`, etc.).
Do not use raw ABI headers under `abi/`.

Type operations are registered via `make_type<kName>()` builder. VDFs via
`make_func<&fn>("name")` builder. Both registered through
`VEF_GENERATE_ENTRY_POINTS(make_extension().type(TYPE)...)`.

## Testing

```bash
cd /path/to/villagesql/build/mysql-test
perl mysql-test-run.pl --suite=/path/to/vsql-boolean/mysql-test
```

Record updated results:

```bash
perl mysql-test-run.pl --suite=/path/to/vsql-boolean/mysql-test --record
```

## Code Style

- File naming: lowercase with underscores (`vsql_boolean.cc`)
- No file-scope `using namespace` — use `using vsql::CustomArg;` declarations
- All SQL entry points wrapped in `try/catch (...)`
- NULL check first in every function body
- Copyright header (GPL-2.0) on all `.cc`, `.h`, `CMakeLists.txt` files
