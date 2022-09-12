#ifndef GCC_TM_H
#define GCC_TM_H
#ifdef IN_GCC
# include "config/aarch64/biarchlp64.h"
# include "config/aarch64/aarch64.h"
# include "config/dbxelf.h"
# include "config/elfos.h"
# include "config/aarch64/aarch64-elf.h"
#if L4_LIBGCC_VERSION >= 10
# include "config/aarch64/aarch64-errata.h"
#endif
# include "config/initfini-array.h"
#endif
# include "defaults.h"
#endif /* GCC_TM_H */
