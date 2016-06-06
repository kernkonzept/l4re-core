//#include <sysdeps/generic/sysdep.h>
#ifdef	__ASSEMBLER__
/* Define an entry point visible from C.  */
#ifdef PIC
#define ENTRY(name)                      \
  .pic										\
  .align 2;                              \
  .globl C_SYMBOL_NAME(name);            \
  .func  C_SYMBOL_NAME(name);            \
  .type  C_SYMBOL_NAME(name), @function; \
C_SYMBOL_NAME(name):
#else
#define ENTRY(name)                      \
  .align 2;                              \
  .globl C_SYMBOL_NAME(name);            \
  .func  C_SYMBOL_NAME(name);            \
  .type  C_SYMBOL_NAME(name), @function; \
C_SYMBOL_NAME(name):
#endif

#undef END
#define END(name) \
  .endfunc;           \
  .size C_SYMBOL_NAME(name), .-C_SYMBOL_NAME(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef HAVE_ELF
	#undef NO_UNDERSCORES
	#define NO_UNDERSCORES
#endif

#ifdef NO_UNDERSCORES
	#define syscall_error __syscall_error
#endif

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#define __do_syscall(syscall_name)		\
  syscall	SYS_ify(syscall_name);

#define SYSCALL_ERROR_HANDLER
#define SYSCALL_ERROR __syscall_error


#ifdef PIC
#ifdef __NDS32_N1213_43U1H__
#ifdef NDS_ABI_V0
#define PSEUDO(name, syscall_name, args)	\
  .pic;										\
  .align 2;					\
  1:	ret;	\
  99:	addi	$r0,	$lp,	0;	\
	jal	1b;	\
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_);	\
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_+4);	\
	add	$r1,	$lp,	$r1;	\
	addi	$lp,	$r0,	0;	\
	sethi $r15, hi20(SYSCALL_ERROR@PLT);	\
	ori	$r15,	$r15, lo12(SYSCALL_ERROR@PLT);	\
	add	$r15, $r15, $r1;	\
	jr		$r15;	\
	nop;                                   	\
	ENTRY(name);                          	\
	__do_syscall(syscall_name);            	\
	bgez $r5, 2f;				\
	sltsi	$r0, $r5, -4096;		\
	beqz	$r0, 99b;			\
  2:
#else
#define PSEUDO(name, syscall_name, args)	\
  .pic;										\
  .align 2;					\
  1:	ret;	\
  99:	addi	$r2,	$lp,	0;	\
	jal	1b;	\
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_);	\
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_+4);	\
	add	$r1,	$lp,	$r1;	\
	addi	$lp,	$r2,	0;	\
	sethi $r15, hi20(SYSCALL_ERROR@PLT);	\
	ori	$r15,	$r15, lo12(SYSCALL_ERROR@PLT);	\
	add	$r15, $r15, $r1;	\
	jr		$r15;	\
	nop;                                   	\
	ENTRY(name);                          	\
	__do_syscall(syscall_name);            	\
	bgez $r0, 2f;				\
	sltsi	$r1, $r0, -4096;		\
	beqz	$r1, 99b;			\
  2:
#endif
#else
#define PSEUDO(name, syscall_name, args)	\
  .pic;										\
  .align 2;					\
  99:	mfusr $r15, $PC;	\
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_ + 4);	\
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_ + 8);	\
	add	$r1,	$r15,	$r1;	\
	sethi $r15, hi20(SYSCALL_ERROR@PLT);	\
	ori	$r15,	$r15, lo12(SYSCALL_ERROR@PLT);	\
	add	$r15, $r15, $r1;	\
	jr		$r15;	\
	nop;                                   	\
	ENTRY(name);                          	\
	__do_syscall(syscall_name);            	\
	bgez $r0, 2f;				\
	sltsi	$r1, $r0, -4096;		\
	beqz	$r1, 99b;			\
  2:
#endif
#else
#ifdef OLD2_ABI
#define PSEUDO(name, syscall_name, args)	\
  .align 2;					\
  99:	j SYSCALL_ERROR;                  	\
	nop;                                   	\
	ENTRY(name);                          	\
	__do_syscall(syscall_name);            	\
        bgez $r5, 2f;                           \
        sltsi   $r0, $r5, -4096;                \
        beqz    $r0, 99b;                       \
  2:
#else
#define PSEUDO(name, syscall_name, args)	\
  .align 2;					\
  99:	j SYSCALL_ERROR;                  	\
	nop;                                   	\
	ENTRY(name);                          	\
	__do_syscall(syscall_name);            	\
        bgez $r0, 2f;                           \
        sltsi   $r1, $r0, -4096;                \
        beqz    $r1, 99b;                       \
  2:
#endif
#endif


#define PSEUDO_NOERRNO(name, syscall_name, args) \
  ENTRY(name);                                   \
  __do_syscall(syscall_name);

#undef PSEUDO_END
#define PSEUDO_END(sym)				\
	SYSCALL_ERROR_HANDLER			\
	END(sym)

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym) END(sym)

#define PSEUDO_ERRVAL(name, syscall_name, args) PSEUDO_NOERRNO(name, syscall_name, args)

#define ret_ERRVAL ret

#define ret_NOERRNO ret	
#if defined NOT_IN_libc
	#define SYSCALL_ERROR __local_syscall_error
	#ifdef PIC
		#ifdef __NDS32_N1213_43U1H__
			#ifdef NDS_ABI_V0
				#define SYSCALL_ERROR_HANDLER				\
				__local_syscall_error:	pushm	$gp, $lp, $sp;				\
					jal	1f;	\
					sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_);	\
					ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+4);	\
					add	$gp,	$gp,	$lp;	\
					neg	$r5, $r5;	\
					push	$r5;				\
					addi	$sp,	$sp, -28; \
					bal	C_SYMBOL_NAME(__errno_location@PLT);	\
					addi	$sp,	$sp, 28; \
					pop	$r1;			\
					swi	$r1, [$r5];				\
					li		$r5, -1;				\
					popm	$gp, $lp, $sp;				\
				1:	ret;
			#else
				#define SYSCALL_ERROR_HANDLER				\
					__local_syscall_error:	pushm	$gp, $lp, $sp;				\
						jal	1f;	\
						sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_);	\
						ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+4);	\
						add	$gp,	$gp,	$lp;	\
						neg	$r0, $r0;	\
						push	$r0;				\
					#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
					        addi    $sp,    $sp, -4; \
					#else \
						addi	$sp,	$sp, -28; \
					#endif \
						bal	C_SYMBOL_NAME(__errno_location@PLT);	\
					#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
					        addi    $sp,    $sp, 4; \
					#else \
						addi	$sp,	$sp, 28; \
					#endif \
						pop	$r1;			\
						swi	$r1, [$r0];				\
						li		$r0, -1;				\
						popm	$gp, $lp, $sp;				\
					1:	ret;
			#endif
		#else
			#define SYSCALL_ERROR_HANDLER				\
				__local_syscall_error:	pushm	$gp, $lp, $sp;				\
					mfusr $r15, $PC;	\
					sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_+4);	\
					ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+8);	\
					add	$gp,	$gp,	$r15;	\
					neg	$r0, $r0;	\
					push	$r0;				\
					#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
					        addi    $sp,    $sp, -4; \
					#else \
						addi	$sp,	$sp, -28; \
					#endif \
					bal	C_SYMBOL_NAME(__errno_location@PLT);	\
					#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
					        addi    $sp,    $sp, 4; \
					#else \
						addi	$sp,	$sp, 28; \
					#endif \
					pop	$r1;			\
					swi	$r1, [$r0];				\
					li		$r0, -1;				\
					popm	$gp, $lp, $sp;				\
				1:	ret;
		#endif
	#else
		#ifdef NDS_ABI_V0
			#define SYSCALL_ERROR_HANDLER	\
			__local_syscall_error:	push	$lp;				\
				neg	$r5, $r5;	\
				push	$r5;				\
				addi	$sp,	$sp, -28; \
				bal	C_SYMBOL_NAME(__errno_location);	\
				addi	$sp,	$sp, 28; \
				pop	$r1;			\
				swi	$r1, [$r5];				\
				li		$r5, -1;				\
				pop	$lp;				\
				ret;
		#else
			#define SYSCALL_ERROR_HANDLER	\
			__local_syscall_error:	push	$lp;				\
				neg	$r0, $r0;	\
				push	$r0;				\
				#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
				        addi    $sp,    $sp, -4; \
				#else \
					addi	$sp,	$sp, -28; \
				#endif \
				bal	C_SYMBOL_NAME(__errno_location);	\
				#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP) \
				        addi    $sp,    $sp, 4; \
				#else \
					addi	$sp,	$sp, 28; \
				#endif \
				pop	$r1;			\
				swi	$r1, [$r0];				\
				li		$r0, -1;				\
				pop	$lp;				\
				ret;
		#endif
	#endif

#else
	#define SYSCALL_ERROR_HANDLER
	#define SYSCALL_ERROR __syscall_error
#endif
#endif	/* __ASSEMBLER__ */
