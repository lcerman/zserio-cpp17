# General notes

- every task ends with all tests of `linux64-gcc` and `linux64-clang` passing,  verification run
  1. `scripts/build.sh cpp_rt-linux64-gcc` passed (build + tests + coverage).
  2. `scripts/build.sh cpp_rt-linux64-clang` passed (build + tests + coverage).


# Builds, tests

The packages `cpp_rt-linux64-gcc` and `cpp_rt-linux64-clang` depend on package `zserio` which can be built using:

`scripts/build.sh zserio`

Building packages with debug ON and test coverage ON (see scripts/build-env.sh) triggers tests to be executed
after building, e.g.,

`scripts/build.sh cpp_rt-linux64-gcc`

or

`scripts/build.sh cpp_rt-linux64-clang`

not only build the lib but also execute tests and computes coverage. Which can be then found in
`build/runtime_lib/linux64-gcc/debug/coverage/gcc` or in `build/runtime_lib/linux64-clang/debug/coverage`.
