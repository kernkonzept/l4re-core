# Libunwind (LLVM)
This package contains the libunwind library from LLVM together with make files
for building them inside the L4Re tree.

We maintain one libunwind version per LLVM major version, assuming that there
are no breaking changes in libunwind within LLVM minor versions, neither in
terms of forward nor backward compatibility.

## Package structure
The `contrib` directory contains unmodified directories from each supported
major LLVM version.

The `build` directory contains the build infrastructure to build libunwind
with L4Re's BID build system.

The libunwind_llvm package is split into two packages:

#### libunwind_llvm
Main package, contains contrib code and build infrastructure.
Provides static (`libunwind_llvm.a`) variant of the libunwind_llvm.

#### libunwind_llvm-pure
Provides static variant of libunwind_llvm without L4 specific functionality
(`libunwind_llvm-pure.a`).

## GCC compatible unwind interface
The header file (`unwind.h`) specifying the GCC compatible unwind interface for
libunwind_llvm, as required by libstdc++ for example, is not part of this
library, but is provided by Clang itself.
