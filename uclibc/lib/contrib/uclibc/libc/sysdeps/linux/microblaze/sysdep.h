#include <common/sysdep.h>

#ifdef  __ASSEMBLER__

/* Syntactic details of assembler.  */

# define ALIGNARG(log2) log2
# define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* Define an entry point visible from C.  */
# define ENTRY(name)                          \
  .globl C_SYMBOL_NAME(name);                 \
  .type C_SYMBOL_NAME(name),@function;        \
  .align ALIGNARG(2);                         \
  C_LABEL(name)

# undef END
# define END(name) ASM_SIZE_DIRECTIVE(name)

/* Local label name for asm code.  */
# ifndef L
#  define L(name) $L##name
# endif

#endif
