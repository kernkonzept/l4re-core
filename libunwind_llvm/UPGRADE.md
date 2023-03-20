# Updating libunwind_llvm
The purpose of this document is to describe what to consider when updating
libunwind_llvm.

## Versioning
The current minor version of libunwind_llvm for each supported major LLVM
version is tracked in a file named `VERSION` in the corresponding contrib
directory.

## Upgrade steps

#### 1. Copy contrib files
Copy the following files from your LLVM source tree into a new contrib directory:

```
libunwind/
```

#### 2. Diff against baseline
Diff your new contrib directory with an existing contrib directory that you
want to use as the baseline, that is, for minor version updates, the contrib
directory of the same major version, and for major version updates, the
contrib directory of the previous major version.

#### 3. Bump max version
If you added a new major version, bump `UNWIND_LLVM_CONTRIB_MAX_VERSION` in
`contrib.inc`.

#### 4. Create or update version file
Create or update a `VERSION` file in the (new) contrib directory.
