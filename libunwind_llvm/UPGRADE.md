# Updating libunwind_llvm
The purpose of this document is to describe what to consider when updating
libunwind_llvm.

## Versioning
The current version of libunwind_llvm is tracked in a file named `VERSION` in
the contrib directory. Unlike with libclang_rt, where we maintain one version
for each LLVM major version, we use a common version of libunwind_llvm for all
LLVM versions.

## Upgrade steps

#### 1. Copy contrib files
Copy the following files from your LLVM source tree into the contrib directory:

```
libunwind/
```

#### 2. Diff against baseline
Diff the changes in the contrib directory and adjust `Makefile.build`
accordingly. If you need to make L4 specific modification in the contrib
directory, please create a corresponding patch file in the `patch` directory to
document that change.

#### 3. Update version file
Update a `VERSION` file in the contrib directory.
