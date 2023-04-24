#ifndef GCC_TM_H
#define GCC_TM_H
#ifdef IN_GCC
# include "insn-modes.h"
# include "insn-constants.h"
#if L4_LIBGCC_VERSION <= 12
# include "config/dbxelf.h"
#endif
# include "config/elfos.h"
# include "config/mips/mips.h"
# include "config/initfini-array.h"
#endif
# include "defaults.h"
#endif /* GCC_TM_H */
