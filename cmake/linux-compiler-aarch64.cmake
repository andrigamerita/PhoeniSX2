set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_COMPILER_TARGET "aarch64-linux-gnu")
set(CMAKE_C_COMPILER_AR "/usr/bin/llvm-ar")
set(CMAKE_C_COMPILER_RANLIB "/usr/bin/llvm-ranlib")

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_COMPILER_TARGET "aarch64-linux-gnu")
set(CMAKE_CXX_COMPILER_AR "/usr/bin/llvm-ar")
set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/llvm-ranlib")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

set(CMAKE_FIND_ROOT_PATH "/opt/aarch64-chroot")
set(CMAKE_SYSROOT "/opt/aarch64-chroot")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
