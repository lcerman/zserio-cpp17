# General notes

- The coding style must be checked by clang-format. The coding style is defined by the configuration file ./.clang-format.
- The quality style must be checked by clang-tidy. The quality style is defined by the configuration file ./runtime/ClangTidyConfig.txt.
- All clang-tidy findings must be fixed if it is possible.
- Using NOLINT directive is forbidden. 
- The quality style for runtime library can have exceptions defined by the suppression file ./runtime/ClangTidySuppressions.txt.
- The reasoning of each such exception must be written directly in the suppression file (as a comment).
- The quality style for integration tests is checked but the compilation does not fail in case of any findings.

# Builds and tests for Linux

- The following environment variables must be set before building or testing:
  - LLVM_PROFDATA_BIN=llvm-profdata-18
  - LLVM_COV_BIN=llvm-cov-18
  - SANITIZERS_ENABLED=1
  - CLANG_TIDY_BIN=clang-tidy
  - CLANG_FORMAT_BIN=clang-format
  - CMAKE_EXTRA_ARGS='-DCMAKE_BUILD_TYPE=Debug'
  - MAKE_CMAKE_GENERATOR="Eclipse CDT4 - Unix Makefiles"
- The clang-18 version must be used.
- The runtime library can be built and tested using the command `scripts/build.sh cpp_rt-linux64-clang`.

# Others

- After the runtime library build using clang, the complete test coverage report can be found at distr/runtime_lib/zserio_doc/coverage/clang-18.
