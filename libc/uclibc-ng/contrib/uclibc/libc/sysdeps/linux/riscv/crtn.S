#include <sys/asm.h>

.macro FUNC_END
  REG_L ra, (sp)
  addi sp, sp, 16
  ret
.endm

.section .init
.global  _init
.type    _init, @function
  FUNC_END


.section .fini
.global  _fini
.type    _fini, @function
  FUNC_END
