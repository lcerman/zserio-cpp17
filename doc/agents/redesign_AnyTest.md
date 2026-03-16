# AnyTest Redesign

## Scope

- Runtime source stays unchanged. The redesign is limited to [AnyTest.cpp](/home/mrozloznik/ZserioCpp17Gama/runtime/test/zserio/AnyTest.cpp).
- The goal is to make `AnyTest::testAny` follow the public `BasicAny` API order from [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h) and to close the remaining reachable coverage gaps in `src/zserio/Any.h`.

## Coverage

- Baseline from the checked-in clang coverage report at [index.html](/home/mrozloznik/ZserioCpp17Gama/distr/runtime_lib/zserio_doc/coverage/clang-18/index.html): `Any.h` line coverage `97.85% (318/325)`, branch coverage `91.67% (44/48)`.
- Post-change coverage: not regenerated in this sandbox. The full sanitizer-enabled test sweep aborts under LeakSanitizer because the sandbox runs under ptrace, so the new `Any.h` percentages still need to be collected outside this environment or with leak detection disabled for the sweep.
- Targeted `llvm-cov` verification from the `AnyTest.*` run confirms execution of [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L106), [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L523), [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L537), and [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L546).

## API Mapping

| BasicAny API / branch | Previous helper | Current helper | Representative types | Coverage goal | Notes |
| --- | --- | --- | --- | --- | --- |
| `BasicAny()` | `testEmptyConstructor` | `testEmptyConstructor` | `int`, `SmallObject`, `BigObject`, `std::vector<int>` | Empty default construction | First step in `testAny`. |
| `BasicAny(const ALLOC&)` | `testAllocatorConstructor` | `testAllocatorConstructor` | same matrix | Empty construction with allocator | Verifies allocator retention. |
| `BasicAny(T&&, const ALLOC&)` lvalue path | mixed constructor coverage | `testLvalueConstructor` | full matrix | Copy value construction | Checks copied value and allocator. |
| `BasicAny(T&&, const ALLOC&)` rvalue path | mixed constructor coverage | `testRvalueConstructor` | full matrix | Move value construction | `std::vector<int>` verifies storage reuse. |
| `BasicAny(const BasicAny&)` | `testCopyConstructor` | `testCopyConstructor` | full matrix | Copy constructor semantics | Verifies `select_on_container_copy_construction`. |
| `BasicAny(const BasicAny&, const ALLOC&)` | `testCopyConstructorWithAllocator` | `testCopyConstructorWithAllocator` | full matrix | Allocator-extended copy construction | Empty-copy case now uses the correct overload. |
| `operator=(const BasicAny&)` | `testCopyAssignmentOperator` | `testCopyAssignmentOperator` | full matrix | Copy assignment semantics | Checks allocator propagation and non-propagation. |
| `BasicAny(BasicAny&&)` | `testMoveConstructor` | `testMoveConstructor` | full matrix | Move construction | Verifies moved-from object is empty. |
| `BasicAny(BasicAny&&, const ALLOC&)` | `testMoveConstructorWithAllocator` | `testMoveConstructorWithAllocator` | full matrix | Allocator-extended move construction | Empty move case now uses the correct overload. |
| `operator=(BasicAny&&)` | `testMoveAssignmentValueOperator` | `testMoveAssignmentOperator` | full matrix | Move assignment semantics | Uses a real non-const move source and checks allocator traits. |
| `operator=(T&&)` | `testMoveAssignmentValueOperator` | `testValueAssignmentOperator` | full matrix | Value assignment | Covers replacing empty and non-empty holder states. |
| `swap(BasicAny&)` fast heap path | partial in old `testSwap` | `testSwap` | `BigObject` | Heap-holder swap with equal allocators | Verifies pointer exchange by address stability. |
| `swap(BasicAny&)` fallback move path | partial in old `testSwap` | `testSwap` | `int`, matrix type | Swap with empty or different-type holder | Exercises temporary-move fallback path. |
| `swap(BasicAny&)` allocator propagation branch | not covered explicitly | `swapPropagatingAllocator` | `BigObject` | `propagate_on_container_swap` branch | Uses `TrackingAllocatorSwapProp`. |
| `reset()` | `testReset` | `testReset` | full matrix | Holder clearing | Verifies reset and reuse. |
| `set(T&&)` normal path | `testSetGet` | `testSet` | full matrix plus `int` and `float` replacement | Value set and holder reuse | Covers same-type and type-changing replacement. |
| `set(T&&)` exception-safety path | not covered explicitly | `testGetIf`, `testConstGetIf` | `ThrowingOnCopyConstruction` | Reach `get_if()` catch paths | Public-API path leaves typed holder present with disengaged `optional`. |
| `emplace<T>(ARGS&&...)` standard path | `testEmplace` | `testEmplace` | full matrix | Direct construction | Verifies constructed value is returned and stored. |
| `HolderBase::construct()` allocator-arg branch | not covered | `testAllocatorArgEmplace` | `AllocatorArgObject<ALLOC>` | Cover [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L104) | Forces `std::allocator_arg_t` construction. |
| `get<T>()` | part of `testSetGet` | `testGet` | full matrix | Non-const get success and bad type | Explicit API-level test. |
| `const get<T>()` | indirect | `testConstGet` | full matrix | Const get success and bad type | Explicit const overload coverage. |
| `get_if<T>()` mismatch and success | `testGetIf` | `testGetIf` | full matrix | Non-const pointer API | Covers empty, success, mismatch and reset. |
| `get_if<T>()` catch branch | not covered | `testGetIf` | `ThrowingOnCopyConstruction` | Cover [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L523) | Triggered after failed `set()`. |
| `const get_if<T>()` mismatch and success | indirect | `testConstGetIf` | full matrix | Const pointer API | Explicit const overload coverage. |
| `const get_if<T>()` catch branch | not covered | `testConstGetIf` | `ThrowingOnCopyConstruction` | Cover [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h#L546) | Triggered after failed `set()`. |
| `isType<T>()` | `testIsType` | `testIsType` | full matrix | Type identity checks | Covers empty, match, mismatch and replacement. |
| `hasValue()` | `testHasValue` | `testHasValue` | full matrix | Presence checks | Covers empty, set, replace and reset. |
| Heap-holder unexpected calls | `unexceptedCallsHeapHolder` | `unexceptedCallsHeapHolder` | `BigObject` | Error branch regression | Keeps explicit holder-level coverage. |
| In-place-holder unexpected calls | `unexceptedCallsNonHeapHolder` | `unexceptedCallsNonHeapHolder` | `uint8_t` | Error branch regression | Keeps explicit holder-level coverage. |
| Shared-library type identity | `sharedLib` | `sharedLib` | runtime test lib values | Type id stability across shared boundary | Unchanged regression coverage. |
| Pretty-function fallback identity | `prettyFunction` | `prettyFunction` | `std::vector<int>` | Type identity regression | Unchanged regression coverage. |

## Notes

- `testAny` is ordered to match the public member declaration order in [Any.h](/home/mrozloznik/ZserioCpp17Gama/runtime/src/zserio/Any.h).
- `ThrowingOnCopyConstruction` is intentionally used instead of touching private storage so the `get_if()` catch branches are covered through public exception-safety behavior.
- Targeted verification passed with `ASAN_OPTIONS=detect_leaks=0 LSAN_OPTIONS=detect_leaks=0` and `--gtest_filter='AnyTest.*'` against `/tmp/zserio-anytest/build/runtime_lib/linux64-clang/debug/runtime_test/ZserioCpp17RuntimeTest`.
- The remaining coverage status should be taken from the regenerated clang report in `/tmp/zserio-anytest/distr/runtime_lib/zserio_doc/coverage/clang-18` once the full test sweep can run without the LeakSanitizer sandbox limitation.
