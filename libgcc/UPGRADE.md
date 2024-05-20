# Updating libgcc
The purpose of this document is to describe what to consider when updating
libgcc.

## Versioning
The current minor version of libgcc for each supported major GCC version is
tracked in a file named `VERSION` in the corresponding contrib directory.

## Upgrade steps

#### 1. Copy contrib files
Copy the following files from your GCC source tree into a new contrib directory:

```
# Libgcc
# Exclude all subdirectories in libgcc/config/ except the following:
#   aarch64, arm, i386, mips, riscv, sparc
libgcc/

# Aarch64
gcc/common/config/aarch64/cpuinfo.h
gcc/config/aarch64/aarch64-arches.def
gcc/config/aarch64/aarch64-cores.def
gcc/config/aarch64/aarch64-elf.h
gcc/config/aarch64/aarch64-errata.h
gcc/config/aarch64/aarch64-opts.h
gcc/config/aarch64/aarch64.h
gcc/config/aarch64/biarchlp64.h

# ARM
gcc/config/arm/arm-flags.h
gcc/config/arm/arm-opts.h
gcc/config/arm/arm.h
gcc/config/arm/bpabi.h
gcc/config/arm/elf.h

# RISC-V
gcc/config/riscv/riscv-opts.h
gcc/config/riscv/riscv.h

# i386
gcc/common/config/i386/cpuinfo.h
gcc/common/config/i386/i386-cpuinfo.h
gcc/config/i386/att.h
gcc/config/i386/biarch64.h
gcc/config/i386/i386-opts.h
gcc/config/i386/i386.h
gcc/config/i386/stringop.def
gcc/config/i386/unix.h
gcc/config/i386/x86-64.h
gcc/config/i386/x86-tune.def

# MIPS
gcc/config/mips/mips-opts.h
gcc/config/mips/mips.h

# Common
gcc/config.in
gcc/config.gcc
gcc/configure.ac
gcc/config/dbxelf.h
gcc/config/elfos.h
gcc/config/initfini-array.h
gcc/config/vxworks-dummy.h
gcc/coretypes.h
gcc/defaults.h
gcc/tsystem.h
include/COPYING
include/COPYING3
include/ansidecl.h
include/dwarf2.def
include/dwarf2.h
include/filenames.h
include/hashtab.h
include/longlong.h

# Meta
COPYING
COPYING.LIB
COPYING.RUNTIME
COPYING3
COPYING3.LIB
README
```

#### 2. Diff against baseline
Diff your new contrib directory with an existing contrib directory that you
want to use as the baseline, that is, for minor version updates, the contrib
directory of the same major version, and for major version updates, the
contrib directory of the previous major version.

For a cleaner diff, it is recommended to use the following regex as a line
filter in order to hide irrelevant copyright header changes:
`Copyright \(C\)[ \t\d-]+`

Of particular interest are changes to the following files:

* `libgcc/Makefile.in`:
  Contains architecture-independent build rules for libgcc, adjust
  `build/src/Makefile.build` accordingly.

* `libgcc/config.host`:
  Contains architecture-specific build configuration, adjust
  `build/src/Makefile-<architecture>` and
  `generated/<architecture>/libgcc/libgcc_tm.h` (generated from `tm_file`) of
  the affected architectures accordingly.

* `libgcc/config/*`:
  Contains both architecture-independent as well as architecture-specific
  build rule fragments, adjust `build/src/libgcc/config/*` accordingly.

* `libgcc/config.in` and `libgcc/configure.ac`:
  Contains architecture-specific feature flags, adjust
  `generated/<architecture>/libgcc/auto-target.h` of the affected
  architectures accordingly.

* `gcc/config.gcc`:
  Contains architecture-specific build configuration, adjust
  `generated/<architecture>/gcc/tm.h` (generated from `tm_file`) of the
  affected architectures accordingly.

* `gcc/config.in` and `gcc/configure.ac`:
  Contains architecture-specific feature flags, adjust
  `generated/<architecture>/gcc/auto-host.h` of the affected architectures
  accordingly.

#### 3. Bump max version
If you added a new major version, bump `LIBGCC_CONTRIB_MAX_VERSION` in
`contrib.inc`.

#### 4. Create or update version file
Create or update a `VERSION` file in the (new) contrib directory.

#### 5. Update file list
If you had to add any additional contrib files please don't forget to update the
list at point 1.
