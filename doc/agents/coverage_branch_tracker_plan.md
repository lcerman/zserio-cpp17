# Branch Coverage Baseline Tracker (GCC + Clang)

## Summary
Create a tracker at `doc/agents/coverage_branch_tracker.md` that lists files needing branch coverage improvement, using immutable baseline values and mutable achieved values for both toolchains.

## Scope and Locked Decisions
- Granularity is per file.
- File set is all files where either GCC or Clang baseline branch coverage is below 100%.
- Coverage sources:
  - GCC baseline from `build/runtime_lib/linux64-gcc/debug` coverage data.
  - Clang baseline from `build/runtime_lib/linux64-clang/debug/coverage/clang/coverage_report.txt`.
- Output tracker path is `doc/agents/coverage_branch_tracker.md`.
- Columns per file:
  - `baseline_gcc_branch`
  - `baseline_clang_branch`
  - `achieved_gcc_branch` (initial `--`)
  - `achieved_clang_branch` (initial `--`)
- Baseline columns stay fixed until task closure.

## Implementation Plan
1. Capture/refresh baseline coverage in debug mode:
   1. `export CMAKE_EXTRA_ARGS='-DCMAKE_BUILD_TYPE=Debug'`
   2. `export GCOVR_BIN=gcovr && scripts/build.sh cpp_rt-linux64-gcc`
   3. `export LLVM_PROFDATA_BIN=llvm-profdata && export LLVM_COV_BIN=llvm-cov && scripts/build.sh cpp_rt-linux64-clang`
2. Extract per-file GCC branch baseline from build coverage data (gcovr branch metric, excluding `.*test_object.*`).
3. Extract per-file Clang branch baseline from `coverage_report.txt` (`Branches` coverage column).
4. Normalize paths to repo-relative keys and merge gcc+clang rows by file.
5. Treat Clang branch cover `-` (no branches) as `100.00%` for filtering consistency.
6. Filter to files needing improvement:
   1. `baseline_gcc_branch < 100%` OR `baseline_clang_branch < 100%`.
7. Sort rows by path ascending.
8. Generate `doc/agents/coverage_branch_tracker.md` with:
   1. Title and baseline capture timestamp.
   2. Legend stating baseline values are immutable.
   3. Summary with tracked file count and overall branch totals for gcc/clang.
   4. Checkbox rows in exact format:
      - `- [ ] <path> | baseline_gcc: XX.XX% | baseline_clang: YY.YY% | achieved_gcc: -- | achieved_clang: --`
9. Validate output:
   1. No duplicate paths.
   2. Every row has all four values.
   3. No files where both baselines are 100%.
10. End-of-task verification runs (required by `AGENTS.md`):
   1. `scripts/build.sh cpp_rt-linux64-gcc`
   2. `scripts/build.sh cpp_rt-linux64-clang`

## Test Cases and Verification
1. Spot-check extracted gcc and clang values for several files against source reports.
2. Confirm filter logic includes files where only one toolchain is below 100%.
3. Confirm tracker is checkbox-based and has baseline/achieved split for both toolchains.
4. Confirm both required verification builds pass.

## Assumptions and Defaults
- Branch coverage is the only metric used for tracking.
- Baseline represents starting task values and remains unchanged.
- Achieved values are edited over time as coverage work progresses.
- Any file missing from one toolchain report is still retained if present in the other, with value handling made explicit in generated output.
