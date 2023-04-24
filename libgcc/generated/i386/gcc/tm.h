#ifndef GCC_TM_H
#define GCC_TM_H
#ifdef IN_GCC
# include "insn-constants.h"
# include "config/i386/biarch64.h"
# include "config/i386/i386.h"
# include "config/i386/unix.h"
# include "config/i386/att.h"
#if L4_LIBGCC_VERSION <= 12
# include "config/dbxelf.h"
#endif
# include "config/elfos.h"
# include "config/i386/x86-64.h"
# include "config/initfini-array.h"
#endif
# include "defaults.h"
#endif /* GCC_TM_H */
