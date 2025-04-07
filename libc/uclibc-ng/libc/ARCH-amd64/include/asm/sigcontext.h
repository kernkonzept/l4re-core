
struct _fpstate {
	unsigned short cwd;
	unsigned short swd;
	unsigned short twd; /* Note this is not the same as the 32bit/x87/FSAVE twd */
	unsigned short fop;
	unsigned long long rip;
	unsigned long long rdp; 
	unsigned int  mxcsr;
	unsigned int  mxcsr_mask;
	unsigned int  st_space[32];	/* 8*16 bytes for each FP-reg */
	unsigned int  xmm_space[64];	/* 16*16 bytes for each XMM-reg  */
	unsigned int  reserved2[24];
};

struct sigcontext { 
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long r11;
	unsigned long r12;
	unsigned long r13;
	unsigned long r14;
	unsigned long r15;
	unsigned long rdi;
	unsigned long rsi;
	unsigned long rbp;
	unsigned long rbx;
	unsigned long rdx;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rsp;
	unsigned long rip;
	unsigned long eflags;		/* RFLAGS */
	unsigned short cs;
	unsigned short gs;
	unsigned short fs;
	unsigned short __pad0;
	unsigned long err;
	unsigned long trapno;
	unsigned long oldmask;
	unsigned long cr2;
	struct _fpstate *fpstate;	/* zero when no FPU context */
	unsigned long reserved1[8];
};
