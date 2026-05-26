# Testing

## Requirements

- VillageSQL built and running (see `AGENTS.md`)
- `VillageSQL_BUILD_DIR` pointing to your VillageSQL build directory
- Extension built and installed via `bash build.sh`

## Build and Install

```bash
VillageSQL_BUILD_DIR=/path/to/villagesql/build bash build.sh
```

This builds `vsql_boolean.veb` and installs it to the VEB directory. The test
runner installs and uninstalls the extension automatically — no manual
`INSTALL EXTENSION` step is needed before running tests.

## Running the Full Suite

**Linux:**
```bash
cd $HOME/build/villagesql/mysql-test
perl mysql-test-run.pl --suite=/path/to/vsql-boolean/mysql-test
```

**macOS:**
```bash
cd ~/.villagesql/build/mysql-test
perl mysql-test-run.pl --suite=/path/to/vsql-boolean/mysql-test
```

## Regenerating Expected Results

After any change to the extension behavior or test queries, regenerate:

```bash
cd /path/to/villagesql/build/mysql-test
perl mysql-test-run.pl --suite=/path/to/vsql-boolean/mysql-test --record
```

Commit both the `.test` and updated `.result` files together.

## Test Files

| File | What it covers |
|---|---|
| `mysql-test/t/boolean_basic.test` | Standard true/false input; integer aliases 1/0; common aliases t/f/yes/no/on/off; NULL handling; WHERE filtering; SHOW CREATE TABLE type display; ORDER BY sorting |
