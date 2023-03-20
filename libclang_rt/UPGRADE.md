# Updating libclang_rt
The purpose of this document is to describe what to consider when updating
libclang_rt.

## Versioning
The current minor version of libclang_rt for each supported major Clang version
is tracked in a file named `VERSION` in the corresponding contrib directory.

## Upgrade steps

#### 1. Copy contrib files
Copy the following files from your LLVM source tree into a new contrib directory:

```
# builtins
# Exclude all subdirectories in compiler-rt/lib/builtins/ except the following:
#   aarch64, arm, i386, ppc, riscv, x86_64
compiler-rt/lib/builtins/

# crt
compiler-rt/lib/crt/
```

#### 2. Diff against baseline
Diff your new contrib directory with an existing contrib directory that you
want to use as the baseline, that is, for minor version updates, the contrib
directory of the same major version, and for major version updates, the
contrib directory of the previous major version.

#### 3. Bump max version
If you added a new major version, bump `LIBCLANG_RT_CONTRIB_MAX_VERSION` in
`contrib.inc`.

#### 4. Create or update version file
Create or update a `VERSION` file in the (new) contrib directory.
