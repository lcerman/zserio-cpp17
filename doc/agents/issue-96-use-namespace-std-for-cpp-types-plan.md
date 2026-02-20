# Implement Issue #96: Use `std`-Namespaced C++ Types Across C++17 Generator + Runtime

## Summary
Apply a repo-wide migration from unqualified standard typedefs to explicitly namespaced forms (`::std::...`) in generator output, runtime code, and regenerated runtime test objects.

## Public API / Interface Impact
- Generated/runtime C++ APIs will use `::std::size_t`, `::std::uint8_t`, `::std::int64_t`, etc.
- No behavior change intended; type spellings and includes only.

## Detailed Implementation Plan
1. Update generator mappings in `extension/src/zserio/extension/cpp17/TypesContext.java` and `extension/src/zserio/extension/cpp17/CppNativeMapper.java` to emit `::std::uint8_t` allocator arguments.
2. Update affected FreeMarker templates under `extension/freemarker/` to emit `::std::size_t`, `::std::ptrdiff_t`, and `::std::{u,}int*_t`.
3. Ensure generated files include `<cstddef>` / `<cstdint>` when those types are emitted (prefer centralized macro changes in `extension/freemarker/FileHeader.inc.ftl`).
4. Update hand-written runtime code in `runtime/src/zserio/**` to use `::std::`-qualified standard typedefs and add missing includes.
5. Regenerate runtime test objects using `scripts/update_test_objects.sh` so checked-in generated files under `runtime/test/test_object/**` reflect new type spellings.
6. Resolve compile fallout (missing includes / namespace-specific edge cases) without semantic changes.
7. Run final verification (required success criteria):
   - `./test.sh cpp-linux64-clang`
   - `./test.sh cpp-linux64-gcc`

## Test Cases and Scenarios
- Build/test pass for both required `./test.sh` targets.
- Regenerated runtime test objects compile for std/pmr/ppmr allocators.
- Spot-check generated output for `::std::allocator<::std::uint8_t>`, `::std::size_t`, and proper `<cstddef>/<cstdint>` coverage.

## Assumptions and Defaults
- Full-repo scope is retained (generator + runtime + generated runtime test objects).
- Verification commands above are the definitive acceptance gate.
