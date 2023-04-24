#ifndef GCC_TM_H
#define GCC_TM_H
#ifdef IN_GCC
#if L4_LIBGCC_VERSION <= 12
# include "config/dbxelf.h"
#endif
# include "config/elfos.h"
# include "config/arm/elf.h"
# include "config/arm/bpabi.h"
# include "config/arm/arm.h"
# include "config/initfini-array.h"
#endif
# include "defaults.h"
#endif /* GCC_TM_H */
