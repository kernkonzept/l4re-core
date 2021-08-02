#include <sys/asm.h>

.macro FUNC_START
  addi sp, sp, -16
  REG_S ra, (sp)
.endm

.section .init
.global  _init
.type    _init, @function
_init:
  FUNC_START

.section .fini
.global  _fini
.type    _fini, @function
_fini:
  FUNC_START
