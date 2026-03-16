# Redesign runtime tests plan

## Summary

Create a tracker at `doc/agents/redesign_AnyTest.md` that lists changed runtime library files and their branch coverage improvement for clang.

## Implementation Plan

1. Check the unit test implementation of the source runtime/src/zserio/Any.h given by the source runtime/test/zserio/AnyTest.cpp.
2. Inspect AnyTest::testAny and create a table in `doc/agents/redesign_AnyTest.md` where the method in BasicAny class will be mapped to the test method in AnyTest::testAny.
  2.1. Make sure that the ordering in AnyTest::testAny is the same with the Basic Any class.
  2.2. If some methods are missing, implement the corresponding method in AnyTest::testAny.
3. Inspect the coverage report at distr/runtime_lib/zserio_doc/coverage/clang-18 for the AnyTest module.
  3.1. Identify all uncovered lines and branches and improve the methods in AnyTest::testAny to have 100% lines and branch coverage.
  3.2. If 100% lines and branch coverage is not possible, report reasoning why to the `doc/agents/redesign_AnyTest.md`.
4. After each change, ask for permission to do it.

## Verification

1. Check the clang build, clang format and clang tidy after changes.