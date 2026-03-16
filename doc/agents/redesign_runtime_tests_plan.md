# Redesign runtime tests plan

## Summary

Create a tracker at `doc/agents/redesign_runtime_tests.md` that lists changed runtime library files and their branch coverage improvement for clang.

## Implementation Plan

1. Check the full implementation of unit tests for runtime library.
2. Each sources and header without source must be properly tested except for the headers stored in the directories runtime/src/zserio/pmr and runtime/src/zserio/ppmr.
3. Each test must has the same name with suffix 'Test' as the tested source.
4. Tests must be stored in the directory runtime/test/zserio.
5. Each test must properly call and check all methods of the tested class with the same ordering.
6. Check if the unit tests ordering is correct. If not change the ordering.
7. Implement all missing unit tests.
8. Unit tests must be as simple as possible but cover all the corner cases. If some corner case is not tested, please describe what and why in the tracker file `doc/agents/redesign_runtime_tests.md`.

## Test Cases and Verification

1. After changes, don't forget to run all GitHub actions stored in the directory ./.github/workflows. All these GitHub actions must pass.