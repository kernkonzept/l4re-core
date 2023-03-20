# Libclang_rt
This package contains the libclang_rt library (LLVM compiler runtime) from Clang
together with make files for building them inside the L4Re tree.

We maintain one libclang_rt version per Clang major version, assuming that there
are no breaking changes in libclang_rt within Clang minor versions, neither in
terms of forward nor backward compatibility.

## Package structure
The `contrib` directory contains unmodified directories from each supported
major Clang version, but not in there entirety, only the subset of files
required to compile libclang_rt.

The `build` directory contains the build infrastructure to build libclang_rt
with L4Re's BID build system.

The libclang_rt package is split into three packages:

#### libclang_rt
Main package, contains contrib code and build infrastructure.
Provides static (`libclang_rt-builtins.a`) variant of the libclang_rt.

#### libclang_rt-pure
Provides static variant of libclang_rt without L4 specific functionality
(`libclang_rt-builtins-pure.a`).

Required when building l4sys, to break a circular dependency, because
libclang_rt depends on l4sys for headers included by libpthread, but at the same
time l4sys relies on functionality from the libclang_rt.

In addition, libclang_rt-pure is used by components such as bootstrap that do
not run in an L4 environment.

#### libclang_rt-crt
Provides C runtime library object files (`crt<...>.o`), used by ldscripts
package.
